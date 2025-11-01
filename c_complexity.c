/*
 * c_complexity.c
 * -----------------------------------------------------------------------------
 * Purpose:
 *   Heuristic, line-by-line complexity analyzer for C sources that emits a
 *   Markdown report to stdout. It totals LOC, classifies lines (code/comment/
 *   preproc/blank), assigns a complexity score to each code line, and estimates
 *   a simple cyclomatic complexity per function.
 *
 * Usage:
 *   gcc -std=c11 -O2 -Wall -Wextra -o c_complexity c_complexity.c
 *   ./c_complexity your_file.c > complexity-your_file.md
 *
 * Output Sections:
 *   1) Summary (LOC, code/comment/preproc/blank, total & average complexity)
 *   2) Per-function cyclomatic complexity (rough heuristic)
 *   3) Per-line complexity table (first PER_LINE_LIMIT lines)
 *
 * Philosophy:
 *   - NOT a full C parser; it is a robust heuristic useful for spotting hot
 *     spots, deep nesting, and dense control logic.
 *   - Safe string handling (bounded ops), consistent memory ownership, and
 *     clear comments for maintainability.
 *
 * Portability Notes:
 *   - We request POSIX prototypes via _POSIX_C_SOURCE.
 *   - We ALWAYS use local fallbacks my_strnlen/my_strdup so there are no
 *     implicit declaration warnings even on older libcs or stricter headers.
 *
 * Tuning Knobs (search for "WEIGHT_" and "PER_LINE_LIMIT"):
 *   - You can adjust scoring weights and the per-line output cap.
 */

#define _POSIX_C_SOURCE 200809L  /* Request POSIX interfaces (e.g., strdup, strnlen) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

/* --------------------------------------------------------------------------
 * Global Configuration (tuning knobs)
 * -------------------------------------------------------------------------- */

/* How many lines of per-line detail to print (avoid gigantic Markdown files) */
#ifndef PER_LINE_LIMIT
#define PER_LINE_LIMIT 500
#endif

/* Max raw input line length we will read from the file */
#ifndef LINE_MAX_CHARS
#define LINE_MAX_CHARS 8192
#endif

/* Column width for previewing original source in the per-line table */
#ifndef PREVIEW_CHARS
#define PREVIEW_CHARS 100
#endif

/* Scoring weights (heuristic) */
#ifndef WEIGHT_BASE
#define WEIGHT_BASE 1      /* base cost per code line */
#endif
#ifndef WEIGHT_CTRL
#define WEIGHT_CTRL 1      /* each control keyword (if/for/while/switch/case/default/goto/else) */
#endif
#ifndef WEIGHT_TERNARY
#define WEIGHT_TERNARY 1   /* ?: presence */
#endif
#ifndef WEIGHT_LOGICAL
#define WEIGHT_LOGICAL 1   /* each && or || */
#endif
#ifndef WEIGHT_INCDEC
#define WEIGHT_INCDEC 1    /* each ++ or -- */
#endif
#ifndef WEIGHT_CALL_PER
#define WEIGHT_CALL_PER 1  /* function call contribution per call (capped) */
#endif
#ifndef WEIGHT_CALL_CAP
#define WEIGHT_CALL_CAP 3  /* cap function call contribution per line */
#endif
#ifndef WEIGHT_DEPTH_MIN
#define WEIGHT_DEPTH_MIN 1 /* minimum added cost when nesting depth > 0 */
#endif

/* --------------------------------------------------------------------------
 * Types
 * -------------------------------------------------------------------------- */

typedef enum { L_BLANK = 0, L_CODE, L_COMMENT, L_PREPROC } LineKind;

typedef struct {
    int   lineno;    /* 1-based line number */
    int   depth;     /* brace nesting depth BEFORE this line’s opens are applied */
    int   score;     /* heuristic complexity score for this line */
    char* flags;     /* comma-separated tags explaining score sources */
    char* text;      /* trimmed preview (<= PREVIEW_CHARS) */
} LineReport;

typedef struct {
    char  name[128];   /* function name (best-effort extraction) */
    int   start_line;  /* inclusive */
    int   end_line;    /* inclusive */
    int   cyclomatic;  /* heuristic cyclomatic complexity */
} FuncReport;

/* --------------------------------------------------------------------------
 * Local portability helpers (used unconditionally to avoid implicit decls)
 * -------------------------------------------------------------------------- */

/* my_strnlen: bounded strlen that stops at max */
static size_t my_strnlen(const char* s, size_t max) {
    size_t n = 0;
    if (!s) return 0;
    while (n < max && s[n] != '\0') n++;
    return n;
}

/* my_strdup: allocate a fresh copy of a C-string using malloc */
static char* my_strdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* --------------------------------------------------------------------------
 * Small string utilities
 * -------------------------------------------------------------------------- */

static int is_ident_char(int c) {
    return isalnum(c) || c == '_';
}

/* Check that s[start..end-1] is a whole word, not part of a larger identifier */
static int is_word_boundary(const char* s, int start, int end) {
    if (start > 0 && is_ident_char((unsigned char)s[start - 1])) return 0;
    if (s[end] && is_ident_char((unsigned char)s[end])) return 0;
    return 1;
}

/* Count occurrences of a whole word (e.g., "if") with identifier boundaries */
static int count_word(const char* s, const char* word) {
    int n = 0;
    int len = (int)strlen(word);
    for (int i = 0; s[i]; ++i) {
        if (!strncmp(&s[i], word, len)) {
            if (is_word_boundary(s, i, i + len)) n++;
        }
    }
    return n;
}

/* Count raw substring occurrences (e.g., "&&", "||") */
static int count_substr(const char* s, const char* pat) {
    int n = 0, len = (int)strlen(pat);
    if (len == 0) return 0;
    for (int i = 0; s[i]; ++i) {
        if (!strncmp(&s[i], pat, len)) n++;
    }
    return n;
}

/* Mask out string and char literals to avoid counting tokens inside them.
   Replaces their contents with spaces so indexing and other tokens remain aligned. */
static void strip_strings(const char* in, char* out, size_t outsz) {
    size_t w = 0;
    int in_str = 0, in_chr = 0, esc = 0;
    for (size_t i = 0; in[i] && w + 1 < outsz; ++i) {
        char c = in[i];
        if (in_str) {
            if (esc) { out[w++] = ' '; esc = 0; }
            else if (c == '\\') { out[w++] = ' '; esc = 1; }
            else if (c == '"')  { out[w++] = ' '; in_str = 0; }
            else { out[w++] = ' '; }
        } else if (in_chr) {
            if (esc) { out[w++] = ' '; esc = 0; }
            else if (c == '\\') { out[w++] = ' '; esc = 1; }
            else if (c == '\'') { out[w++] = ' '; in_chr = 0; }
            else { out[w++] = ' '; }
        } else {
            if (c == '"')       { out[w++] = ' '; in_str = 1; }
            else if (c == '\'') { out[w++] = ' '; in_chr = 1; }
            else { out[w++] = c; }
        }
    }
    out[w] = '\0';
}

/* --------------------------------------------------------------------------
 * Line classification: code / comment / preprocessor / blank
 * Tracks C-style block comments across lines via static state.
 * -------------------------------------------------------------------------- */

static int g_in_block_comment = 0;

static LineKind classify_line(const char* raw) {
    /* Trim left for quick checks */
    const char* s = raw;
    while (*s && isspace((unsigned char)*s)) s++;

    if (*s == '\0') return L_BLANK;

    if (g_in_block_comment) {

        const char* end = strstr(s, "*/");
        if (end) g_in_block_comment = 0;
        return L_COMMENT;
    }

    /* Start of a new block comment (without closing on this same line) */
    const char* start = strstr(s, "/*");
    const char* end = strstr(s, "*/");
    if (start && (!end || end < start)) {
        g_in_block_comment = 1;
        return L_COMMENT;
    }

    /* Single-line // comment */
    if (s[0] == '/' && s[1] == '/') return L_COMMENT;

    /* Preprocessor directive */
    if (s[0] == '#') return L_PREPROC;

    /* Otherwise, assume it's code */
    return L_CODE;
}

/* --------------------------------------------------------------------------
 * Function detection (best-effort)
 * - Extract a "name" token immediately preceding '(' on lines that look like
 *   function definitions (have '{' and no ';' before it).
 * - Avoid treating control keywords as function names.
 * -------------------------------------------------------------------------- */

static int is_control_name(const char* name) {
    static const char* kw[] = { "if","for","while","switch","return","sizeof","do","else","case","default" };
    for (size_t i = 0; i < sizeof(kw)/sizeof(kw[0]); ++i) {
        if (strcmp(name, kw[i]) == 0) return 1;
    }
    return 0;
}

/* Attempt to extract a function name from a line.
   This is a heuristic: it finds the token immediately before '(' and checks it. */
static void extract_func_name(const char* line, char out[128]) {
    out[0] = '\0';
    const char* p = strchr(line, '(');
    if (!p) return;

    /* Walk backward over whitespace and '*' to find the end of the name */
    const char* q = p - 1;
    while (q >= line && (isspace((unsigned char)*q) || *q == '*')) q--;
    const char* end = q;

    /* Walk backward to the start of the identifier */
    while (q >= line && is_ident_char((unsigned char)*q)) q--;
    const char* start = q + 1;

    int len = (int)(end - start + 1);
    if (len <= 0 || len >= 120) return;

    strncpy(out, start, (size_t)len);
    out[len] = '\0';

    if (is_control_name(out)) out[0] = '\0';
}

/* --------------------------------------------------------------------------
 * Flag management for per-line explanations
 * -------------------------------------------------------------------------- */

static void append_flag(char** flags, const char* add) {
    if (!add || !*add) return;
    size_t old = (*flags) ? strlen(*flags) : 0;
    size_t addlen = strlen(add);
    size_t newcap = old + addlen + (old ? 2 : 1); /* +1 for comma +1 for '\0' */
    char* buf = (char*)realloc(*flags, newcap);
    if (!buf) return; /* OOM: silently ignore flag growth to avoid crashing */
    *flags = buf;
    if (old == 0) {
        strcpy(*flags, add);
    } else {
        (*flags)[old] = ',';
        strcpy((*flags) + old + 1, add);
    }
}

/* --------------------------------------------------------------------------
 * Per-line scoring function (heuristic)
 * -------------------------------------------------------------------------- */

static int score_line(const char* raw, int depth, char** flags_out) {
    char* flags = NULL;
    int score = 0;

    if (!raw || !*raw) {
        append_flag(&flags, "blank");
        *flags_out = flags;
        return 0;
    }

    /* Base score: each code line "costs" at least this much */
    score += WEIGHT_BASE;

    /* Operate on a version with literals blanked out to avoid false positives */
    char tmp[LINE_MAX_CHARS];
    strip_strings(raw, tmp, sizeof(tmp));

    /* Control keywords: else, if/for/while/switch/case/default/goto */
    int h = 0;
    h = count_word(tmp, "else");
    if (h) { score += h * WEIGHT_CTRL; append_flag(&flags, "else"); }

    const char* ctrls[] = {"if","for","while","switch","case","default","goto"};
    for (size_t i = 0; i < sizeof(ctrls)/sizeof(ctrls[0]); ++i) {
        int c = count_word(tmp, ctrls[i]);
        if (c) { score += c * WEIGHT_CTRL; append_flag(&flags, ctrls[i]); }
    }

    /* Ternary ?: (very rough presence detection) */
    if (strchr(tmp, '?') && strchr(tmp, ':')) {
        score += WEIGHT_TERNARY;
        append_flag(&flags, "?:");
    }

    /* Logical operators */
    int land = count_substr(tmp, "&&");
    int lor  = count_substr(tmp, "||");
    if (land) { score += land * WEIGHT_LOGICAL; append_flag(&flags, "&&"); }
    if (lor)  { score += lor  * WEIGHT_LOGICAL; append_flag(&flags, "||"); }

    /* Increment/decrement operators */
    int inc = count_substr(tmp, "++");
    int dec = count_substr(tmp, "--");
    if (inc) { score += inc * WEIGHT_INCDEC; append_flag(&flags, "++"); }
    if (dec) { score += dec * WEIGHT_INCDEC; append_flag(&flags, "--"); }

    /* Function calls (cap contribution per line to avoid runaway) */
    int calls = 0;
    for (const char* p = tmp; (p = strchr(p, '(')); ++p) {
        /* Identify token before '(' */
        const char* q = p - 1;
        while (q >= tmp && (isspace((unsigned char)*q) || *q == '*')) q--;
        const char* end = q;
        while (q >= tmp && is_ident_char((unsigned char)*q)) q--;
        int len = (int)(end - (q + 1) + 1);
        if (len > 0 && len < 64) {
            char name[64]; memcpy(name, q + 1, (size_t)len); name[len] = '\0';
            if (!is_control_name(name) && strcmp(name, "return") && strcmp(name, "sizeof")) {
                calls++;
            }
        }
    }
    if (calls > 0) {
        int add = calls * WEIGHT_CALL_PER;
        if (add > WEIGHT_CALL_CAP) add = WEIGHT_CALL_CAP;
        score += add;

        char note[32];
        snprintf(note, sizeof(note), "calls:%d", calls);
        append_flag(&flags, note);
    }

    /* Nesting depth penalty: encourages flatter code */
    if (depth > 0) {
        int add = depth / 2;
        if (add < WEIGHT_DEPTH_MIN) add = WEIGHT_DEPTH_MIN;
        score += add;

        char note[32];
        snprintf(note, sizeof(note), "depth:%d", depth);
        append_flag(&flags, note);
    }

    *flags_out = flags;
    return score;
}

/* --------------------------------------------------------------------------
 * Main analysis routine
 * -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.c> > report.md\n", argv[0]);
        return 1;
    }

    const char* path = argv[1];
    FILE* f = fopen(path, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }

    LineReport* lines = NULL;
    size_t lines_cap = 0, lines_len = 0;

    FuncReport* funcs = NULL;
    size_t funcs_cap = 0, funcs_len = 0;

    int code_cnt = 0, comment_cnt = 0, preproc_cnt = 0, blank_cnt = 0;
    int total_score = 0;
    int depth = 0;

    int lineno = 0;
    int in_func = 0;
    FuncReport current;
    memset(&current, 0, sizeof(current));

    char line[LINE_MAX_CHARS];
    while (fgets(line, sizeof(line), f)) {
        lineno++;

        /* Classify current line BEFORE brace updates */
        LineKind kind = classify_line(line);

        /* Count braces on this line to update depth AFTER scoring */
        int opens = 0, closes = 0;
        for (char* p = line; *p; ++p) {
            if (*p == '{') opens++;
            else if (*p == '}') closes++;
        }

        /* Heuristic function-start detection:
           - This line has '(' and '{'
           - No ';' prior to the first '{' (filter prototypes/macros)
           - Extract a plausible name and exclude control keywords. */
        if (kind == L_CODE && !in_func) {
            char* lbrace = strchr(line, '{');
            char* semi   = strchr(line, ';');
            char* lparen = strchr(line, '(');
            if (lbrace && lparen && (!semi || semi > lbrace)) {
                char name[128];
                extract_func_name(line, name);
                if (name[0]) {
                    in_func = 1;
                    memset(&current, 0, sizeof(current));
                    strncpy(current.name, name, sizeof(current.name) - 1);
                    current.start_line = lineno;
                    current.cyclomatic = 1;  /* baseline */
                }
            }
        }

        /* Tally classification */
        if (kind == L_CODE)      code_cnt++;
        else if (kind == L_COMMENT)  comment_cnt++;
        else if (kind == L_PREPROC)  preproc_cnt++;
        else                     blank_cnt++;

        /* Grow per-line report buffer as needed */
        if (lines_len == lines_cap) {
            size_t newcap = (lines_cap == 0) ? 512 : (lines_cap * 2);
            LineReport* tmp = (LineReport*)realloc(lines, newcap * sizeof(LineReport));
            if (!tmp) { fprintf(stderr, "Out of memory\n"); fclose(f); return 2; }
            lines = tmp; lines_cap = newcap;
        }

        LineReport* lr = &lines[lines_len++];
        lr->lineno = lineno;
        lr->depth  = depth;  /* score reflects depth PRIOR to opening braces on this line */
        lr->score  = 0;
        lr->flags  = NULL;

        /* Create preview snippet for the per-line table (trim leading spaces) */
        const char* s = line;
        while (*s == ' ' || *s == '\t') s++;
        char preview[PREVIEW_CHARS + 1];
        snprintf(preview, sizeof(preview), "%.*s", PREVIEW_CHARS, s);

        if (kind == L_CODE) {
            lr->score = score_line(line, depth, &lr->flags);
            total_score += lr->score;

            /* Cyclomatic bumps inside a function */
            if (in_func) {
                char stripped[LINE_MAX_CHARS];
                strip_strings(line, stripped, sizeof(stripped));
                if (count_word(stripped, "if"))      current.cyclomatic++;
                if (count_word(stripped, "for"))     current.cyclomatic++;
                if (count_word(stripped, "while"))   current.cyclomatic++;
                if (count_word(stripped, "case"))    current.cyclomatic++;
                if (count_word(stripped, "default")) current.cyclomatic++;
                if (strchr(stripped, '?') && strchr(stripped, ':')) current.cyclomatic++;
                current.cyclomatic += count_substr(stripped, "&&");
                current.cyclomatic += count_substr(stripped, "||");
            }

            lr->text = my_strdup(preview);
        } else {
            /* Non-code lines get simple flags and empty preview text */
            const char* k = (kind == L_COMMENT) ? "comment" :
                            (kind == L_PREPROC) ? "preproc" : "blank";
            lr->flags = my_strdup(k);
            lr->text  = my_strdup("");
        }

        /* Update nesting depth AFTER processing/scoring this line */
        depth += opens;
        depth -= closes;
        if (depth < 0) depth = 0;

        /* Function end (heuristic): when overall depth returns to 0 and we saw a close brace */
        if (in_func && depth == 0 && closes > 0) {
            current.end_line = lineno;

            if (funcs_len == funcs_cap) {
                size_t newcap = (funcs_cap == 0) ? 64 : (funcs_cap * 2);
                FuncReport* tmp = (FuncReport*)realloc(funcs, newcap * sizeof(FuncReport));
                if (!tmp) { fprintf(stderr, "Out of memory\n"); fclose(f); return 3; }
                funcs = tmp; funcs_cap = newcap;
            }
            funcs[funcs_len++] = current;

            in_func = 0;
            memset(&current, 0, sizeof(current));
        }
    }

    fclose(f);

    /* Summary calculations */
    int loc = lineno;
    double avg = (code_cnt > 0) ? ((double)total_score / (double)code_cnt) : 0.0;

    /* Emit Markdown report */
    printf("# C Complexity Report\n\n");
    printf("## Summary\n");
    printf("- **Total lines**: %d\n", loc);
    printf("- **Code lines**: %d\n", code_cnt);
    printf("- **Comment lines**: %d\n", comment_cnt);
    printf("- **Preprocessor lines**: %d\n", preproc_cnt);
    printf("- **Blank lines**: %d\n", blank_cnt);
    printf("- **Total line complexity score**: %d\n", total_score);
    printf("- **Average per code line**: %.2f\n", avg);

    if (funcs_len > 0) {
        printf("\n## Per-Function Cyclomatic Complexity (heuristic)\n");
        printf("| Function | Start Line | End Line | Complexity |\n|---|---:|---:|---:|\n");
        for (size_t i = 0; i < funcs_len; i++) {
            printf("| `%s` | %d | %d | %d |\n",
                   funcs[i].name, funcs[i].start_line, funcs[i].end_line, funcs[i].cyclomatic);
        }
    } else {
        printf("\n> No functions detected by the heuristic parser.\n");
    }

    printf("\n## Per-Line Complexity (first %d lines)\n", PER_LINE_LIMIT);
    printf("| # | Depth | Score | Flags | Code |\n|---:|---:|---:|---|---|\n");
    for (size_t i = 0; i < lines_len && i < PER_LINE_LIMIT; i++) {
        LineReport* lr = &lines[i];

        /* Escape '|' in preview for Markdown table cells */
        char esc[(PREVIEW_CHARS * 2) + 4];
        size_t w = 0;
        for (const char* p = lr->text ? lr->text : ""; *p && w + 2 < sizeof(esc); ++p) {
            if (*p == '|') { esc[w++] = '\\'; esc[w++] = '|'; }
            else           { esc[w++] = *p; }
        }
        esc[w] = '\0';

        printf("| %d | %d | %d | %s | `%s` |\n",
               lr->lineno, lr->depth, lr->score, lr->flags ? lr->flags : "", esc);
    }

    if ((int)lines_len > PER_LINE_LIMIT) {
        printf("\n> Truncated to first %d lines out of %zu. "
               "Recompile with a higher PER_LINE_LIMIT to include more.\n",
               PER_LINE_LIMIT, lines_len);
    }

    /* Free all allocated memory */
    for (size_t i = 0; i < lines_len; i++) {
        free(lines[i].flags);
        free(lines[i].text);
    }
    free(lines);
    free(funcs);

    return 0;
}

