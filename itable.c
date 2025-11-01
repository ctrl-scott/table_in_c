// itable.c
// Interactive terminal table with per-row actions using ncurses.
// Keys: ↑/↓/k/j to move, ←/→/h/l to change column, Enter/Space to view,
//       e to edit cell, a to add row, d to delete row, q to quit.

#define _XOPEN_SOURCE 700
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach/mach.h>
#endif


#define MAX_COLS 3
#define COL0_W 6
#define COL1_W 18
#define COL2_W 12

typedef struct {
    int id;
    char name[64];
    char status[32];
} Row;

typedef struct {
    Row* data;
    size_t len;
    size_t cap;
} RowVec;

static void die_cleanup(const char* msg) {
    endwin();
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static void vec_init(RowVec* v) {
    v->data = NULL; v->len = 0; v->cap = 0;
}
static void vec_reserve(RowVec* v, size_t need) {
    if (need <= v->cap) return;
    size_t ncap = v->cap ? v->cap*2 : 8;
    if (ncap < need) ncap = need;
    Row* nd = (Row*)realloc(v->data, ncap * sizeof(Row));
    if (!nd) die_cleanup("Out of memory");
    v->data = nd; v->cap = ncap;
}
static void vec_push(RowVec* v, Row r) {
    vec_reserve(v, v->len+1);
    v->data[v->len++] = r;
}
static void vec_erase(RowVec* v, size_t idx) {
    if (idx >= v->len) return;
    for (size_t i = idx+1; i < v->len; ++i) v->data[i-1] = v->data[i];
    v->len--;
}
static void vec_free(RowVec* v) { free(v->data); v->data=NULL; v->len=v->cap=0; }

static void seed_data(RowVec* v) {
    for (int i=1;i<=25;i++){
        Row r;
        r.id = i;
        snprintf(r.name, sizeof(r.name), "Item %02d", i);
        snprintf(r.status, sizeof(r.status), (i%3==0)?"Pending":(i%3==1)?"Active":"Paused");
        vec_push(v, r);
    }
}

static void draw_border(int top, int left, int width, int height) {
    mvhline(top, left, 0, width);
    mvhline(top+height-1, left, 0, width);
    mvvline(top, left, 0, height);
    mvvline(top, left+width-1, 0, height);
    mvaddch(top, left, ACS_ULCORNER);
    mvaddch(top, left+width-1, ACS_URCORNER);
    mvaddch(top+height-1, left, ACS_LLCORNER);
    mvaddch(top+height-1, left+width-1, ACS_LRCORNER);
}

static int prompt_line_input(int y, int x, int maxlen, const char* prompt, char* buf, int bufsz) {
    echo();
    curs_set(1);
    mvprintw(y, x, "%s", prompt);
    clrtoeol();
    move(y, x + (int)strlen(prompt));
    int res = wgetnstr(stdscr, buf, bufsz-1);
    noecho();
    curs_set(0);
    return (res == OK) ? 0 : -1;
}

static void show_message_center(const char* msg) {
    int h,w; getmaxyx(stdscr, h, w);
    int y = h/2, x = (w - (int)strlen(msg))/2;
    attron(A_BOLD);
    mvprintw(y, x<0?0:x, "%s", msg);
    attroff(A_BOLD);
    refresh();
}

static void show_details(const Row* r) {
    int h,w; getmaxyx(stdscr, h, w);
    int box_w = 40, box_h = 7;
    int top = (h - box_h)/2;
    int left = (w - box_w)/2;
    // backdrop
    attron(A_DIM);
    for (int i=0;i<h;i++){ mvhline(i, 0, ' ', w); }
    attroff(A_DIM);

    // modal
    draw_border(top, left, box_w, box_h);
    mvprintw(top+1, left+2, "Row details");
    mvhline(top+2, left+1, ACS_HLINE, box_w-2);
    mvprintw(top+3, left+2, "ID: %d", r->id);
    mvprintw(top+4, left+2, "Name: %s", r->name);
    mvprintw(top+5, left+2, "Status: %s", r->status);
    mvprintw(top+box_h, left+2, " ");
    mvprintw(top+box_h-1, left+2, "Press any key to return");
    refresh();
    getch();
}

static void draw_table(const RowVec* v, size_t sel, size_t col_focus, size_t scroll, int top, int left, int width, int height) {
    // Header
    attron(A_BOLD | A_UNDERLINE);
    mvprintw(top, left, " %-*s %-*s %-*s ",
             COL0_W, "ID",
             COL1_W, "Name",
             COL2_W, "Status");
    attroff(A_BOLD | A_UNDERLINE);

    int rows_area = height - 2; // minus header and footer
    size_t max_visible = (rows_area > 0) ? (size_t)rows_area : 0;

    for (size_t i = 0; i < max_visible; ++i) {
        size_t idx = scroll + i;
        int y = top + 1 + (int)i;
        move(y, left);
        clrtoeol();
        if (idx >= v->len) continue;

        const Row* r = &v->data[idx];
        bool is_sel = (idx == sel);

        if (is_sel) attron(A_REVERSE);
        // draw each column; highlight focused column subtly
        if (is_sel && col_focus == 0) attron(A_BOLD);
        mvprintw(y, left, " %-*d ", COL0_W, r->id);
        if (is_sel && col_focus == 0) attroff(A_BOLD);

        if (is_sel && col_focus == 1) attron(A_BOLD);
        printw(" %-*.*s ", COL1_W, COL1_W, r->name);
        if (is_sel && col_focus == 1) attroff(A_BOLD);

        if (is_sel && col_focus == 2) attron(A_BOLD);
        printw(" %-*.*s ", COL2_W, COL2_W, r->status);
        if (is_sel && col_focus == 2) attroff(A_BOLD);

        if (is_sel) attroff(A_REVERSE);
    }

    // Footer help
    int fy = top + height - 1;
    move(fy, left);
    clrtoeol();
    attron(A_DIM);
    mvprintw(fy, left, " Arrows/kjhl: Move  Enter: View  e: Edit  a: Add  d: Del  q: Quit ");
    attroff(A_DIM);
}

static void edit_cell(Row* r, size_t col, int footer_y, int left) {
    char buf[128];
    int x = left;
    switch (col) {
        case 0: {
            // edit ID as integer
            buf[0]='\0';
            if (prompt_line_input(footer_y, x, 10, "New ID: ", buf, sizeof(buf)) == 0) {
                // parse int
                char* end=NULL; long val = strtol(buf, &end, 10);
                if (end && *end=='\0') r->id = (int)val;
            }
        } break;
        case 1: {
            strncpy(buf, r->name, sizeof(buf)); buf[sizeof(buf)-1]='\0';
            if (prompt_line_input(footer_y, x, (int)sizeof(r->name)-1, "New Name: ", buf, sizeof(buf)) == 0) {
                buf[sizeof(r->name)-1]='\0';
                strncpy(r->name, buf, sizeof(r->name));
                r->name[sizeof(r->name)-1]='\0';
            }
        } break;
        case 2: {
            strncpy(buf, r->status, sizeof(buf)); buf[sizeof(buf)-1]='\0';
            if (prompt_line_input(footer_y, x, (int)sizeof(r->status)-1, "New Status: ", buf, sizeof(buf)) == 0) {
                buf[sizeof(r->status)-1]='\0';
                strncpy(r->status, buf, sizeof(r->status));
                r->status[sizeof(r->status)-1]='\0';
            }
        } break;
    }
}
//added in helpers to output memory
typedef struct {
    unsigned long long rss_bytes;   // Resident Set Size (bytes)
    unsigned long long vsize_bytes; // Virtual memory size (bytes)
    unsigned long long phys_bytes;  // Total physical system memory (bytes)
    unsigned long long lim_as;      // RLIMIT_AS (bytes) or RLIM_INFINITY
    unsigned long long lim_data;    // RLIMIT_DATA (bytes) or RLIM_INFINITY
    unsigned long long lim_stack;   // RLIMIT_STACK (bytes) or RLIM_INFINITY;
    int have_proc;                  // 1 if proc stats were read
} MemInfo;

static const char* human_bytes(unsigned long long b, char* out, size_t n) {
    const char* units[] = {"B","KB","MB","GB","TB","PB"};
    int u = 0;
    double v = (double)b;
    while (v >= 1024.0 && u < 5) { v /= 1024.0; u++; }
    snprintf(out, n, "%.2f %s", v, units[u]);
    return out;
}

#ifdef __linux__
static int read_proc_status_kb(const char* key, unsigned long long* out_kb) {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return 0;
    char line[512];
    int ok = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, strlen(key)) == 0) {
            // Example: "VmRSS:	   12345 kB"
            unsigned long long v = 0ULL;
            if (sscanf(line + strlen(key), ":%*s %llu", &v) == 1) { *out_kb = v; ok = 1; }
            break;
        }
    }
    fclose(f);
    return ok;
}
#endif

static void get_mem_info(MemInfo* mi) {
    memset(mi, 0, sizeof(*mi));

    // Total physical
    long pages = sysconf(_SC_PHYS_PAGES);
    long pgsize = sysconf(_SC_PAGESIZE);
    if (pages > 0 && pgsize > 0) mi->phys_bytes = (unsigned long long)pages * (unsigned long long)pgsize;

    // Limits
    struct rlimit rl;
    mi->lim_as = RLIM_INFINITY; mi->lim_data = RLIM_INFINITY; mi->lim_stack = RLIM_INFINITY;
    if (getrlimit(RLIMIT_AS, &rl) == 0)   mi->lim_as   = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : (unsigned long long)rl.rlim_cur;
    if (getrlimit(RLIMIT_DATA, &rl) == 0) mi->lim_data = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : (unsigned long long)rl.rlim_cur;
    if (getrlimit(RLIMIT_STACK, &rl) == 0)mi->lim_stack= (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : (unsigned long long)rl.rlim_cur;

#ifdef __linux__
    // Prefer /proc for current RSS and VSZ (kB -> bytes)
    unsigned long long rss_kb=0, vms_kb=0;
    mi->have_proc = 0;
    if (read_proc_status_kb("VmRSS", &rss_kb)) { mi->rss_bytes = rss_kb * 1024ULL; mi->have_proc = 1; }
    if (read_proc_status_kb("VmSize", &vms_kb)) { mi->vsize_bytes = vms_kb * 1024ULL; mi->have_proc = 1; }

    if (!mi->have_proc) {
        // Fallback to rusage (ru_maxrss is kB on Linux)
        struct rusage ru;
        if (getrusage(RUSAGE_SELF, &ru) == 0) {
            mi->rss_bytes = (unsigned long long)ru.ru_maxrss * 1024ULL;
        }
    }
#elif defined(__APPLE__)
    // macOS: use Mach task_info for resident/virtual
    task_basic_info_data_t tinfo;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    kern_return_t kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&tinfo, &count);
    if (kr == KERN_SUCCESS) {
        mi->rss_bytes = (unsigned long long)tinfo.resident_size;
        mi->vsize_bytes = (unsigned long long)tinfo.virtual_size;
        mi->have_proc = 1;
    } else {
        // Fallback ru_maxrss is bytes on macOS
        struct rusage ru;
        if (getrusage(RUSAGE_SELF, &ru) == 0) {
            mi->rss_bytes = (unsigned long long)ru.ru_maxrss;
        }
    }
#endif
}

static void format_limit(char* out, size_t n, unsigned long long lim_bytes) {
    if (lim_bytes == RLIM_INFINITY) { snprintf(out, n, "unlimited"); return; }
    human_bytes(lim_bytes, out, n);
}

//Here is main

int main(void) {
    RowVec vec; vec_init(&vec);
    seed_data(&vec);

    if (initscr() == NULL) { fprintf(stderr, "Failed to init ncurses\n"); return 1; }
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    start_color();
    use_default_colors();
	
	MemInfo mem;
	get_mem_info(&mem);

	// Pre-format text once; it will be shown in the header every frame.
	char rss_h[32], vsz_h[32], phys_h[32], as_h[32], data_h[32], stack_h[32];
	human_bytes(mem.rss_bytes, rss_h, sizeof(rss_h));
	human_bytes(mem.vsize_bytes, vsz_h, sizeof(vsz_h));
	human_bytes(mem.phys_bytes, phys_h, sizeof(phys_h));
	format_limit(as_h, sizeof(as_h), mem.lim_as);
	format_limit(data_h, sizeof(data_h), mem.lim_data);
	format_limit(stack_h, sizeof(stack_h), mem.lim_stack);

    size_t sel = 0;            // selected row index
    size_t col_focus = 1;      // 0=ID,1=Name,2=Status
    size_t scroll = 0;

    while (1) {
        erase();

        int h,w; getmaxyx(stdscr, h, w);
        int top = 1, left = 2;
        int box_w = COL0_W + COL1_W + COL2_W + 6; // spaces + margins
        if (box_w + left + 1 > w) box_w = w - left - 1;
        int box_h = h - 2;
        if (box_h < 6) box_h = 6;

/*        mvprintw(0, 2, "Interactive Table (rows: %zu)  |  Selected: %zu  |  Column: %zu", vec.len, sel, col_focus);
        draw_border(top-1, left-1, box_w+2, box_h+2);
        draw_table(&vec, sel, col_focus, scroll, top, left, box_w, box_h);*/
		// header with memory info
		
		mvprintw(0, 2,
		    "Interactive Table (rows: %zu) | Sel:%zu Col:%zu | RSS:%s VSZ:%s | Phys:%s | AS:%s DATA:%s STACK:%s",
		    vec.len, sel, col_focus, rss_h, vsz_h, phys_h, as_h, data_h, stack_h);
	        draw_border(top-1, left-1, box_w+2, box_h+2);
	        draw_table(&vec, sel, col_focus, scroll, top, left, box_w, box_h);
		

        refresh();

        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        int rows_area = box_h - 2;
        if (rows_area < 1) rows_area = 1;
        size_t max_visible = (size_t)rows_area;

        switch (ch) {
            case KEY_UP: case 'k':
                if (sel > 0) sel--;
                if (sel < scroll) scroll = sel;
                break;
            case KEY_DOWN: case 'j':
                if (vec.len == 0) break;
                if (sel + 1 < vec.len) sel++;
                if (sel >= scroll + max_visible) scroll = sel - max_visible + 1;
                break;
            case KEY_LEFT: case 'h':
                if (col_focus > 0) col_focus--;
                break;
            case KEY_RIGHT: case 'l':
                if (col_focus + 1 < MAX_COLS) col_focus++;
                break;
            case 10: // Enter
            case ' ': // Space
                if (vec.len > 0 && sel < vec.len) show_details(&vec.data[sel]);
                break;
            case 'e':
            case 'E':
                if (vec.len > 0 && sel < vec.len) {
                    edit_cell(&vec.data[sel], col_focus, top+box_h+1, left);
                }
                break;
            case 'a':
            case 'A': {
                Row r;
                r.id = (int)(vec.len ? vec.data[vec.len-1].id + 1 : 1);
                strncpy(r.name, "New Item", sizeof(r.name));
                r.name[sizeof(r.name)-1]='\0';
                strncpy(r.status, "Pending", sizeof(r.status));
                r.status[sizeof(r.status)-1]='\0';
                vec_push(&vec, r);
                sel = vec.len ? vec.len-1 : 0;
                if (sel >= scroll + max_visible) scroll = sel - max_visible + 1;
            } break;
            case 'd':
            case 'D':
                if (vec.len > 0 && sel < vec.len) {
                    vec_erase(&vec, sel);
                    if (sel >= vec.len && sel > 0) sel--;
                    if (scroll > sel) scroll = sel;
                }
                break;
            default:
                // ignore
                break;
        }
    }

    endwin();
    vec_free(&vec);
    return 0;
}
