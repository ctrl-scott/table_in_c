# C Complexity Report

## Summary
- **Total lines**: 416
- **Code lines**: 323
- **Comment lines**: 26
- **Preprocessor lines**: 20
- **Blank lines**: 47
- **Total line complexity score**: 923
- **Average per code line**: 2.86

## Per-Function Cyclomatic Complexity (heuristic)
| Function | Start Line | End Line | Complexity |
|---|---:|---:|---:|
| `die_cleanup` | 36 | 40 | 1 |
| `vec_init` | 42 | 44 | 1 |
| `vec_reserve` | 45 | 52 | 5 |
| `vec_push` | 53 | 56 | 1 |
| `vec_erase` | 57 | 61 | 3 |
| `vec_free` | 62 | 62 | 1 |
| `seed_data` | 64 | 72 | 3 |
| `draw_border` | 74 | 83 | 1 |
| `prompt_line_input` | 85 | 95 | 2 |
| `show_message_center` | 97 | 104 | 2 |
| `show_details` | 106 | 127 | 2 |
| `draw_table` | 129 | 175 | 18 |
| `edit_cell` | 177 | 207 | 9 |
| `human_bytes` | 219 | 226 | 3 |
| `read_proc_status_kb` | 229 | 244 | 5 |
| `get_mem_info` | 247 | 293 | 15 |
| `format_limit` | 295 | 298 | 2 |
| `main` | 302 | 416 | 40 |

## Per-Line Complexity (first 500 lines)
| # | Depth | Score | Flags | Code |
|---:|---:|---:|---|---|
| 1 | 0 | 0 | comment | `` |
| 2 | 0 | 0 | comment | `` |
| 3 | 0 | 0 | comment | `` |
| 4 | 0 | 0 | comment | `` |
| 5 | 0 | 0 | blank | `` |
| 6 | 0 | 0 | preproc | `` |
| 7 | 0 | 0 | preproc | `` |
| 8 | 0 | 0 | preproc | `` |
| 9 | 0 | 0 | preproc | `` |
| 10 | 0 | 0 | preproc | `` |
| 11 | 0 | 0 | preproc | `` |
| 12 | 0 | 0 | preproc | `` |
| 13 | 0 | 0 | preproc | `` |
| 14 | 0 | 0 | preproc | `` |
| 15 | 0 | 0 | preproc | `` |
| 16 | 0 | 0 | preproc | `` |
| 17 | 0 | 0 | blank | `` |
| 18 | 0 | 0 | blank | `` |
| 19 | 0 | 0 | preproc | `` |
| 20 | 0 | 0 | preproc | `` |
| 21 | 0 | 0 | preproc | `` |
| 22 | 0 | 0 | preproc | `` |
| 23 | 0 | 0 | blank | `` |
| 24 | 0 | 1 |  | `typedef struct {
` |
| 25 | 1 | 2 | depth:1 | `int id;
` |
| 26 | 1 | 2 | depth:1 | `char name[64];
` |
| 27 | 1 | 2 | depth:1 | `char status[32];
` |
| 28 | 1 | 2 | depth:1 | `} Row;
` |
| 29 | 0 | 0 | blank | `` |
| 30 | 0 | 1 |  | `typedef struct {
` |
| 31 | 1 | 2 | depth:1 | `Row* data;
` |
| 32 | 1 | 2 | depth:1 | `size_t len;
` |
| 33 | 1 | 2 | depth:1 | `size_t cap;
` |
| 34 | 1 | 2 | depth:1 | `} RowVec;
` |
| 35 | 0 | 0 | blank | `` |
| 36 | 0 | 2 | calls:1 | `static void die_cleanup(const char* msg) {
` |
| 37 | 1 | 3 | calls:1,depth:1 | `endwin();
` |
| 38 | 1 | 3 | calls:1,depth:1 | `fprintf(stderr, "%s\n", msg);
` |
| 39 | 1 | 3 | calls:1,depth:1 | `exit(EXIT_FAILURE);
` |
| 40 | 1 | 2 | depth:1 | `}
` |
| 41 | 0 | 0 | blank | `` |
| 42 | 0 | 2 | calls:1 | `static void vec_init(RowVec* v) {
` |
| 43 | 1 | 2 | depth:1 | `v->data = NULL; v->len = 0; v->cap = 0;
` |
| 44 | 1 | 2 | depth:1 | `}
` |
| 45 | 0 | 2 | calls:1 | `static void vec_reserve(RowVec* v, size_t need) {
` |
| 46 | 1 | 3 | if,depth:1 | `if (need <= v->cap) return;
` |
| 47 | 1 | 3 | ?:,depth:1 | `size_t ncap = v->cap ? v->cap*2 : 8;
` |
| 48 | 1 | 3 | if,depth:1 | `if (ncap < need) ncap = need;
` |
| 49 | 1 | 3 | calls:1,depth:1 | `Row* nd = (Row*)realloc(v->data, ncap * sizeof(Row));
` |
| 50 | 1 | 4 | if,calls:1,depth:1 | `if (!nd) die_cleanup("Out of memory");
` |
| 51 | 1 | 2 | depth:1 | `v->data = nd; v->cap = ncap;
` |
| 52 | 1 | 2 | depth:1 | `}
` |
| 53 | 0 | 2 | calls:1 | `static void vec_push(RowVec* v, Row r) {
` |
| 54 | 1 | 3 | calls:1,depth:1 | `vec_reserve(v, v->len+1);
` |
| 55 | 1 | 3 | ++,depth:1 | `v->data[v->len++] = r;
` |
| 56 | 1 | 2 | depth:1 | `}
` |
| 57 | 0 | 2 | calls:1 | `static void vec_erase(RowVec* v, size_t idx) {
` |
| 58 | 1 | 3 | if,depth:1 | `if (idx >= v->len) return;
` |
| 59 | 1 | 4 | for,++,depth:1 | `for (size_t i = idx+1; i < v->len; ++i) v->data[i-1] = v->data[i];
` |
| 60 | 1 | 3 | --,depth:1 | `v->len--;
` |
| 61 | 1 | 2 | depth:1 | `}
` |
| 62 | 0 | 3 | calls:2 | `static void vec_free(RowVec* v) { free(v->data); v->data=NULL; v->len=v->cap=0; }
` |
| 63 | 0 | 0 | blank | `` |
| 64 | 0 | 2 | calls:1 | `static void seed_data(RowVec* v) {
` |
| 65 | 1 | 4 | for,++,depth:1 | `for (int i=1;i<=25;i++){
` |
| 66 | 2 | 2 | depth:2 | `Row r;
` |
| 67 | 2 | 2 | depth:2 | `r.id = i;
` |
| 68 | 2 | 3 | calls:1,depth:2 | `snprintf(r.name, sizeof(r.name), "Item %02d", i);
` |
| 69 | 2 | 4 | ?:,calls:1,depth:2 | `snprintf(r.status, sizeof(r.status), (i%3==0)?"Pending":(i%3==1)?"Active":"Paused");
` |
| 70 | 2 | 3 | calls:1,depth:2 | `vec_push(v, r);
` |
| 71 | 2 | 2 | depth:2 | `}
` |
| 72 | 1 | 2 | depth:1 | `}
` |
| 73 | 0 | 0 | blank | `` |
| 74 | 0 | 2 | calls:1 | `static void draw_border(int top, int left, int width, int height) {
` |
| 75 | 1 | 3 | calls:1,depth:1 | `mvhline(top, left, 0, width);
` |
| 76 | 1 | 3 | calls:1,depth:1 | `mvhline(top+height-1, left, 0, width);
` |
| 77 | 1 | 3 | calls:1,depth:1 | `mvvline(top, left, 0, height);
` |
| 78 | 1 | 3 | calls:1,depth:1 | `mvvline(top, left+width-1, 0, height);
` |
| 79 | 1 | 3 | calls:1,depth:1 | `mvaddch(top, left, ACS_ULCORNER);
` |
| 80 | 1 | 3 | calls:1,depth:1 | `mvaddch(top, left+width-1, ACS_URCORNER);
` |
| 81 | 1 | 3 | calls:1,depth:1 | `mvaddch(top+height-1, left, ACS_LLCORNER);
` |
| 82 | 1 | 3 | calls:1,depth:1 | `mvaddch(top+height-1, left+width-1, ACS_LRCORNER);
` |
| 83 | 1 | 2 | depth:1 | `}
` |
| 84 | 0 | 0 | blank | `` |
| 85 | 0 | 2 | calls:1 | `static int prompt_line_input(int y, int x, int maxlen, const char* prompt, char* buf, int bufsz) {
` |
| 86 | 1 | 3 | calls:1,depth:1 | `echo();
` |
| 87 | 1 | 3 | calls:1,depth:1 | `curs_set(1);
` |
| 88 | 1 | 3 | calls:1,depth:1 | `mvprintw(y, x, "%s", prompt);
` |
| 89 | 1 | 3 | calls:1,depth:1 | `clrtoeol();
` |
| 90 | 1 | 4 | calls:2,depth:1 | `move(y, x + (int)strlen(prompt));
` |
| 91 | 1 | 3 | calls:1,depth:1 | `int res = wgetnstr(stdscr, buf, bufsz-1);
` |
| 92 | 1 | 3 | calls:1,depth:1 | `noecho();
` |
| 93 | 1 | 3 | calls:1,depth:1 | `curs_set(0);
` |
| 94 | 1 | 3 | ?:,depth:1 | `return (res == OK) ? 0 : -1;
` |
| 95 | 1 | 2 | depth:1 | `}
` |
| 96 | 0 | 0 | blank | `` |
| 97 | 0 | 2 | calls:1 | `static void show_message_center(const char* msg) {
` |
| 98 | 1 | 3 | calls:1,depth:1 | `int h,w; getmaxyx(stdscr, h, w);
` |
| 99 | 1 | 3 | calls:1,depth:1 | `int y = h/2, x = (w - (int)strlen(msg))/2;
` |
| 100 | 1 | 3 | calls:1,depth:1 | `attron(A_BOLD);
` |
| 101 | 1 | 4 | ?:,calls:1,depth:1 | `mvprintw(y, x<0?0:x, "%s", msg);
` |
| 102 | 1 | 3 | calls:1,depth:1 | `attroff(A_BOLD);
` |
| 103 | 1 | 3 | calls:1,depth:1 | `refresh();
` |
| 104 | 1 | 2 | depth:1 | `}
` |
| 105 | 0 | 0 | blank | `` |
| 106 | 0 | 2 | calls:1 | `static void show_details(const Row* r) {
` |
| 107 | 1 | 3 | calls:1,depth:1 | `int h,w; getmaxyx(stdscr, h, w);
` |
| 108 | 1 | 2 | depth:1 | `int box_w = 40, box_h = 7;
` |
| 109 | 1 | 2 | depth:1 | `int top = (h - box_h)/2;
` |
| 110 | 1 | 2 | depth:1 | `int left = (w - box_w)/2;
` |
| 111 | 1 | 0 | comment | `` |
| 112 | 1 | 3 | calls:1,depth:1 | `attron(A_DIM);
` |
| 113 | 1 | 5 | for,++,calls:1,depth:1 | `for (int i=0;i<h;i++){ mvhline(i, 0, ' ', w); }
` |
| 114 | 1 | 3 | calls:1,depth:1 | `attroff(A_DIM);
` |
| 115 | 1 | 0 | blank | `` |
| 116 | 1 | 0 | comment | `` |
| 117 | 1 | 3 | calls:1,depth:1 | `draw_border(top, left, box_w, box_h);
` |
| 118 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+1, left+2, "Row details");
` |
| 119 | 1 | 3 | calls:1,depth:1 | `mvhline(top+2, left+1, ACS_HLINE, box_w-2);
` |
| 120 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+3, left+2, "ID: %d", r->id);
` |
| 121 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+4, left+2, "Name: %s", r->name);
` |
| 122 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+5, left+2, "Status: %s", r->status);
` |
| 123 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+box_h, left+2, " ");
` |
| 124 | 1 | 3 | calls:1,depth:1 | `mvprintw(top+box_h-1, left+2, "Press any key to return");
` |
| 125 | 1 | 3 | calls:1,depth:1 | `refresh();
` |
| 126 | 1 | 3 | calls:1,depth:1 | `getch();
` |
| 127 | 1 | 2 | depth:1 | `}
` |
| 128 | 0 | 0 | blank | `` |
| 129 | 0 | 2 | calls:1 | `static void draw_table(const RowVec* v, size_t sel, size_t col_focus, size_t scroll, int top, int le` |
| 130 | 1 | 0 | comment | `` |
| 131 | 1 | 3 | calls:1,depth:1 | `attron(A_BOLD \| A_UNDERLINE);
` |
| 132 | 1 | 3 | calls:1,depth:1 | `mvprintw(top, left, " %-*s %-*s %-*s ",
` |
| 133 | 1 | 2 | depth:1 | `COL0_W, "ID",
` |
| 134 | 1 | 2 | depth:1 | `COL1_W, "Name",
` |
| 135 | 1 | 2 | depth:1 | `COL2_W, "Status");
` |
| 136 | 1 | 3 | calls:1,depth:1 | `attroff(A_BOLD \| A_UNDERLINE);
` |
| 137 | 1 | 0 | blank | `` |
| 138 | 1 | 2 | depth:1 | `int rows_area = height - 2; // minus header and footer
` |
| 139 | 1 | 3 | ?:,depth:1 | `size_t max_visible = (rows_area > 0) ? (size_t)rows_area : 0;
` |
| 140 | 1 | 0 | blank | `` |
| 141 | 1 | 4 | for,++,depth:1 | `for (size_t i = 0; i < max_visible; ++i) {
` |
| 142 | 2 | 2 | depth:2 | `size_t idx = scroll + i;
` |
| 143 | 2 | 2 | depth:2 | `int y = top + 1 + (int)i;
` |
| 144 | 2 | 3 | calls:1,depth:2 | `move(y, left);
` |
| 145 | 2 | 3 | calls:1,depth:2 | `clrtoeol();
` |
| 146 | 2 | 3 | if,depth:2 | `if (idx >= v->len) continue;
` |
| 147 | 2 | 0 | blank | `` |
| 148 | 2 | 2 | depth:2 | `const Row* r = &v->data[idx];
` |
| 149 | 2 | 2 | depth:2 | `bool is_sel = (idx == sel);
` |
| 150 | 2 | 0 | blank | `` |
| 151 | 2 | 4 | if,calls:1,depth:2 | `if (is_sel) attron(A_REVERSE);
` |
| 152 | 2 | 0 | comment | `` |
| 153 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 0) attron(A_BOLD);
` |
| 154 | 2 | 3 | calls:1,depth:2 | `mvprintw(y, left, " %-*d ", COL0_W, r->id);
` |
| 155 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 0) attroff(A_BOLD);
` |
| 156 | 2 | 0 | blank | `` |
| 157 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 1) attron(A_BOLD);
` |
| 158 | 2 | 3 | calls:1,depth:2 | `printw(" %-*.*s ", COL1_W, COL1_W, r->name);
` |
| 159 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 1) attroff(A_BOLD);
` |
| 160 | 2 | 0 | blank | `` |
| 161 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 2) attron(A_BOLD);
` |
| 162 | 2 | 3 | calls:1,depth:2 | `printw(" %-*.*s ", COL2_W, COL2_W, r->status);
` |
| 163 | 2 | 5 | if,&&,calls:1,depth:2 | `if (is_sel && col_focus == 2) attroff(A_BOLD);
` |
| 164 | 2 | 0 | blank | `` |
| 165 | 2 | 4 | if,calls:1,depth:2 | `if (is_sel) attroff(A_REVERSE);
` |
| 166 | 2 | 2 | depth:2 | `}
` |
| 167 | 1 | 0 | blank | `` |
| 168 | 1 | 0 | comment | `` |
| 169 | 1 | 2 | depth:1 | `int fy = top + height - 1;
` |
| 170 | 1 | 3 | calls:1,depth:1 | `move(fy, left);
` |
| 171 | 1 | 3 | calls:1,depth:1 | `clrtoeol();
` |
| 172 | 1 | 3 | calls:1,depth:1 | `attron(A_DIM);
` |
| 173 | 1 | 3 | calls:1,depth:1 | `mvprintw(fy, left, " Arrows/kjhl: Move  Enter: View  e: Edit  a: Add  d: Del  q: Quit ");
` |
| 174 | 1 | 3 | calls:1,depth:1 | `attroff(A_DIM);
` |
| 175 | 1 | 2 | depth:1 | `}
` |
| 176 | 0 | 0 | blank | `` |
| 177 | 0 | 2 | calls:1 | `static void edit_cell(Row* r, size_t col, int footer_y, int left) {
` |
| 178 | 1 | 2 | depth:1 | `char buf[128];
` |
| 179 | 1 | 2 | depth:1 | `int x = left;
` |
| 180 | 1 | 3 | switch,depth:1 | `switch (col) {
` |
| 181 | 2 | 3 | case,depth:2 | `case 0: {
` |
| 182 | 3 | 0 | comment | `` |
| 183 | 3 | 2 | depth:3 | `buf[0]='\0';
` |
| 184 | 3 | 4 | if,calls:1,depth:3 | `if (prompt_line_input(footer_y, x, 10, "New ID: ", buf, sizeof(buf)) == 0) {
` |
| 185 | 4 | 0 | comment | `` |
| 186 | 4 | 4 | calls:1,depth:4 | `char* end=NULL; long val = strtol(buf, &end, 10);
` |
| 187 | 4 | 5 | if,&&,depth:4 | `if (end && *end=='\0') r->id = (int)val;
` |
| 188 | 4 | 3 | depth:4 | `}
` |
| 189 | 3 | 2 | depth:3 | `} break;
` |
| 190 | 2 | 3 | case,depth:2 | `case 1: {
` |
| 191 | 3 | 3 | calls:1,depth:3 | `strncpy(buf, r->name, sizeof(buf)); buf[sizeof(buf)-1]='\0';
` |
| 192 | 3 | 4 | if,calls:1,depth:3 | `if (prompt_line_input(footer_y, x, (int)sizeof(r->name)-1, "New Name: ", buf, sizeof(buf)) == 0) {
` |
| 193 | 4 | 3 | depth:4 | `buf[sizeof(r->name)-1]='\0';
` |
| 194 | 4 | 4 | calls:1,depth:4 | `strncpy(r->name, buf, sizeof(r->name));
` |
| 195 | 4 | 3 | depth:4 | `r->name[sizeof(r->name)-1]='\0';
` |
| 196 | 4 | 3 | depth:4 | `}
` |
| 197 | 3 | 2 | depth:3 | `} break;
` |
| 198 | 2 | 3 | case,depth:2 | `case 2: {
` |
| 199 | 3 | 3 | calls:1,depth:3 | `strncpy(buf, r->status, sizeof(buf)); buf[sizeof(buf)-1]='\0';
` |
| 200 | 3 | 4 | if,calls:1,depth:3 | `if (prompt_line_input(footer_y, x, (int)sizeof(r->status)-1, "New Status: ", buf, sizeof(buf)) == 0)` |
| 201 | 4 | 3 | depth:4 | `buf[sizeof(r->status)-1]='\0';
` |
| 202 | 4 | 4 | calls:1,depth:4 | `strncpy(r->status, buf, sizeof(r->status));
` |
| 203 | 4 | 3 | depth:4 | `r->status[sizeof(r->status)-1]='\0';
` |
| 204 | 4 | 3 | depth:4 | `}
` |
| 205 | 3 | 2 | depth:3 | `} break;
` |
| 206 | 2 | 2 | depth:2 | `}
` |
| 207 | 1 | 2 | depth:1 | `}
` |
| 208 | 0 | 0 | comment | `` |
| 209 | 0 | 1 |  | `typedef struct {
` |
| 210 | 1 | 3 | calls:1,depth:1 | `unsigned long long rss_bytes;   // Resident Set Size (bytes)
` |
| 211 | 1 | 3 | calls:1,depth:1 | `unsigned long long vsize_bytes; // Virtual memory size (bytes)
` |
| 212 | 1 | 3 | calls:1,depth:1 | `unsigned long long phys_bytes;  // Total physical system memory (bytes)
` |
| 213 | 1 | 3 | calls:1,depth:1 | `unsigned long long lim_as;      // RLIMIT_AS (bytes) or RLIM_INFINITY
` |
| 214 | 1 | 3 | calls:1,depth:1 | `unsigned long long lim_data;    // RLIMIT_DATA (bytes) or RLIM_INFINITY
` |
| 215 | 1 | 3 | calls:1,depth:1 | `unsigned long long lim_stack;   // RLIMIT_STACK (bytes) or RLIM_INFINITY;
` |
| 216 | 1 | 3 | if,depth:1 | `int have_proc;                  // 1 if proc stats were read
` |
| 217 | 1 | 2 | depth:1 | `} MemInfo;
` |
| 218 | 0 | 0 | blank | `` |
| 219 | 0 | 2 | calls:1 | `static const char* human_bytes(unsigned long long b, char* out, size_t n) {
` |
| 220 | 1 | 2 | depth:1 | `const char* units[] = {"B","KB","MB","GB","TB","PB"};
` |
| 221 | 1 | 2 | depth:1 | `int u = 0;
` |
| 222 | 1 | 2 | depth:1 | `double v = (double)b;
` |
| 223 | 1 | 5 | while,&&,++,depth:1 | `while (v >= 1024.0 && u < 5) { v /= 1024.0; u++; }
` |
| 224 | 1 | 3 | calls:1,depth:1 | `snprintf(out, n, "%.2f %s", v, units[u]);
` |
| 225 | 1 | 2 | depth:1 | `return out;
` |
| 226 | 1 | 2 | depth:1 | `}
` |
| 227 | 0 | 0 | blank | `` |
| 228 | 0 | 0 | preproc | `` |
| 229 | 0 | 2 | calls:1 | `static int read_proc_status_kb(const char* key, unsigned long long* out_kb) {
` |
| 230 | 1 | 3 | calls:1,depth:1 | `FILE* f = fopen("/proc/self/status", "r");
` |
| 231 | 1 | 3 | if,depth:1 | `if (!f) return 0;
` |
| 232 | 1 | 2 | depth:1 | `char line[512];
` |
| 233 | 1 | 2 | depth:1 | `int ok = 0;
` |
| 234 | 1 | 4 | while,calls:1,depth:1 | `while (fgets(line, sizeof(line), f)) {
` |
| 235 | 2 | 5 | if,calls:2,depth:2 | `if (strncmp(line, key, strlen(key)) == 0) {
` |
| 236 | 3 | 0 | comment | `` |
| 237 | 3 | 2 | depth:3 | `unsigned long long v = 0ULL;
` |
| 238 | 3 | 5 | if,calls:2,depth:3 | `if (sscanf(line + strlen(key), ":%*s %llu", &v) == 1) { *out_kb = v; ok = 1; }
` |
| 239 | 3 | 2 | depth:3 | `break;
` |
| 240 | 3 | 2 | depth:3 | `}
` |
| 241 | 2 | 2 | depth:2 | `}
` |
| 242 | 1 | 3 | calls:1,depth:1 | `fclose(f);
` |
| 243 | 1 | 2 | depth:1 | `return ok;
` |
| 244 | 1 | 2 | depth:1 | `}
` |
| 245 | 0 | 0 | preproc | `` |
| 246 | 0 | 0 | blank | `` |
| 247 | 0 | 2 | calls:1 | `static void get_mem_info(MemInfo* mi) {
` |
| 248 | 1 | 3 | calls:1,depth:1 | `memset(mi, 0, sizeof(*mi));
` |
| 249 | 1 | 0 | blank | `` |
| 250 | 1 | 0 | comment | `` |
| 251 | 1 | 3 | calls:1,depth:1 | `long pages = sysconf(_SC_PHYS_PAGES);
` |
| 252 | 1 | 3 | calls:1,depth:1 | `long pgsize = sysconf(_SC_PAGESIZE);
` |
| 253 | 1 | 5 | if,&&,calls:1,depth:1 | `if (pages > 0 && pgsize > 0) mi->phys_bytes = (unsigned long long)pages * (unsigned long long)pgsize` |
| 254 | 1 | 0 | blank | `` |
| 255 | 1 | 0 | comment | `` |
| 256 | 1 | 2 | depth:1 | `struct rlimit rl;
` |
| 257 | 1 | 2 | depth:1 | `mi->lim_as = RLIM_INFINITY; mi->lim_data = RLIM_INFINITY; mi->lim_stack = RLIM_INFINITY;
` |
| 258 | 1 | 5 | if,?:,calls:1,depth:1 | `if (getrlimit(RLIMIT_AS, &rl) == 0)   mi->lim_as   = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY ` |
| 259 | 1 | 5 | if,?:,calls:1,depth:1 | `if (getrlimit(RLIMIT_DATA, &rl) == 0) mi->lim_data = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY ` |
| 260 | 1 | 5 | if,?:,calls:1,depth:1 | `if (getrlimit(RLIMIT_STACK, &rl) == 0)mi->lim_stack= (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY ` |
| 261 | 1 | 0 | blank | `` |
| 262 | 1 | 0 | preproc | `` |
| 263 | 1 | 0 | comment | `` |
| 264 | 1 | 2 | depth:1 | `unsigned long long rss_kb=0, vms_kb=0;
` |
| 265 | 1 | 2 | depth:1 | `mi->have_proc = 0;
` |
| 266 | 1 | 4 | if,calls:1,depth:1 | `if (read_proc_status_kb("VmRSS", &rss_kb)) { mi->rss_bytes = rss_kb * 1024ULL; mi->have_proc = 1; }
` |
| 267 | 1 | 4 | if,calls:1,depth:1 | `if (read_proc_status_kb("VmSize", &vms_kb)) { mi->vsize_bytes = vms_kb * 1024ULL; mi->have_proc = 1;` |
| 268 | 1 | 0 | blank | `` |
| 269 | 1 | 3 | if,depth:1 | `if (!mi->have_proc) {
` |
| 270 | 2 | 0 | comment | `` |
| 271 | 2 | 2 | depth:2 | `struct rusage ru;
` |
| 272 | 2 | 4 | if,calls:1,depth:2 | `if (getrusage(RUSAGE_SELF, &ru) == 0) {
` |
| 273 | 3 | 2 | depth:3 | `mi->rss_bytes = (unsigned long long)ru.ru_maxrss * 1024ULL;
` |
| 274 | 3 | 2 | depth:3 | `}
` |
| 275 | 2 | 2 | depth:2 | `}
` |
| 276 | 1 | 0 | preproc | `` |
| 277 | 1 | 0 | comment | `` |
| 278 | 1 | 2 | depth:1 | `task_basic_info_data_t tinfo;
` |
| 279 | 1 | 2 | depth:1 | `mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
` |
| 280 | 1 | 4 | calls:2,depth:1 | `kern_return_t kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)&tinfo, &count);
` |
| 281 | 1 | 3 | if,depth:1 | `if (kr == KERN_SUCCESS) {
` |
| 282 | 2 | 2 | depth:2 | `mi->rss_bytes = (unsigned long long)tinfo.resident_size;
` |
| 283 | 2 | 2 | depth:2 | `mi->vsize_bytes = (unsigned long long)tinfo.virtual_size;
` |
| 284 | 2 | 2 | depth:2 | `mi->have_proc = 1;
` |
| 285 | 2 | 3 | else,depth:2 | `} else {
` |
| 286 | 2 | 0 | comment | `` |
| 287 | 2 | 2 | depth:2 | `struct rusage ru;
` |
| 288 | 2 | 4 | if,calls:1,depth:2 | `if (getrusage(RUSAGE_SELF, &ru) == 0) {
` |
| 289 | 3 | 2 | depth:3 | `mi->rss_bytes = (unsigned long long)ru.ru_maxrss;
` |
| 290 | 3 | 2 | depth:3 | `}
` |
| 291 | 2 | 2 | depth:2 | `}
` |
| 292 | 1 | 0 | preproc | `` |
| 293 | 1 | 2 | depth:1 | `}
` |
| 294 | 0 | 0 | blank | `` |
| 295 | 0 | 2 | calls:1 | `static void format_limit(char* out, size_t n, unsigned long long lim_bytes) {
` |
| 296 | 1 | 4 | if,calls:1,depth:1 | `if (lim_bytes == RLIM_INFINITY) { snprintf(out, n, "unlimited"); return; }
` |
| 297 | 1 | 3 | calls:1,depth:1 | `human_bytes(lim_bytes, out, n);
` |
| 298 | 1 | 2 | depth:1 | `}
` |
| 299 | 0 | 0 | blank | `` |
| 300 | 0 | 0 | comment | `` |
| 301 | 0 | 0 | blank | `` |
| 302 | 0 | 2 | calls:1 | `int main(void) {
` |
| 303 | 1 | 3 | calls:1,depth:1 | `RowVec vec; vec_init(&vec);
` |
| 304 | 1 | 3 | calls:1,depth:1 | `seed_data(&vec);
` |
| 305 | 1 | 0 | blank | `` |
| 306 | 1 | 5 | if,calls:2,depth:1 | `if (initscr() == NULL) { fprintf(stderr, "Failed to init ncurses\n"); return 1; }
` |
| 307 | 1 | 3 | calls:1,depth:1 | `noecho();
` |
| 308 | 1 | 3 | calls:1,depth:1 | `cbreak();
` |
| 309 | 1 | 3 | calls:1,depth:1 | `keypad(stdscr, TRUE);
` |
| 310 | 1 | 3 | calls:1,depth:1 | `curs_set(0);
` |
| 311 | 1 | 3 | calls:1,depth:1 | `start_color();
` |
| 312 | 1 | 3 | calls:1,depth:1 | `use_default_colors();
` |
| 313 | 1 | 0 | blank | `` |
| 314 | 1 | 2 | depth:1 | `MemInfo mem;
` |
| 315 | 1 | 3 | calls:1,depth:1 | `get_mem_info(&mem);
` |
| 316 | 1 | 0 | blank | `` |
| 317 | 1 | 0 | comment | `` |
| 318 | 1 | 2 | depth:1 | `char rss_h[32], vsz_h[32], phys_h[32], as_h[32], data_h[32], stack_h[32];
` |
| 319 | 1 | 3 | calls:1,depth:1 | `human_bytes(mem.rss_bytes, rss_h, sizeof(rss_h));
` |
| 320 | 1 | 3 | calls:1,depth:1 | `human_bytes(mem.vsize_bytes, vsz_h, sizeof(vsz_h));
` |
| 321 | 1 | 3 | calls:1,depth:1 | `human_bytes(mem.phys_bytes, phys_h, sizeof(phys_h));
` |
| 322 | 1 | 3 | calls:1,depth:1 | `format_limit(as_h, sizeof(as_h), mem.lim_as);
` |
| 323 | 1 | 3 | calls:1,depth:1 | `format_limit(data_h, sizeof(data_h), mem.lim_data);
` |
| 324 | 1 | 3 | calls:1,depth:1 | `format_limit(stack_h, sizeof(stack_h), mem.lim_stack);
` |
| 325 | 1 | 0 | blank | `` |
| 326 | 1 | 2 | depth:1 | `size_t sel = 0;            // selected row index
` |
| 327 | 1 | 2 | depth:1 | `size_t col_focus = 1;      // 0=ID,1=Name,2=Status
` |
| 328 | 1 | 2 | depth:1 | `size_t scroll = 0;
` |
| 329 | 1 | 0 | blank | `` |
| 330 | 1 | 3 | while,depth:1 | `while (1) {
` |
| 331 | 2 | 3 | calls:1,depth:2 | `erase();
` |
| 332 | 2 | 0 | blank | `` |
| 333 | 2 | 3 | calls:1,depth:2 | `int h,w; getmaxyx(stdscr, h, w);
` |
| 334 | 2 | 2 | depth:2 | `int top = 1, left = 2;
` |
| 335 | 2 | 2 | depth:2 | `int box_w = COL0_W + COL1_W + COL2_W + 6; // spaces + margins
` |
| 336 | 2 | 3 | if,depth:2 | `if (box_w + left + 1 > w) box_w = w - left - 1;
` |
| 337 | 2 | 2 | depth:2 | `int box_h = h - 2;
` |
| 338 | 2 | 3 | if,depth:2 | `if (box_h < 6) box_h = 6;
` |
| 339 | 2 | 0 | blank | `` |
| 340 | 2 | 0 | comment | `` |
| 341 | 2 | 0 | comment | `` |
| 342 | 2 | 0 | comment | `` |
| 343 | 2 | 0 | comment | `` |
| 344 | 2 | 0 | blank | `` |
| 345 | 2 | 3 | calls:1,depth:2 | `mvprintw(0, 2,
` |
| 346 | 2 | 2 | depth:2 | `"Interactive Table (rows: %zu) \| Sel:%zu Col:%zu \| RSS:%s VSZ:%s \| Phys:%s \| AS:%s DATA:%s STACK:%s"` |
| 347 | 2 | 2 | depth:2 | `vec.len, sel, col_focus, rss_h, vsz_h, phys_h, as_h, data_h, stack_h);
` |
| 348 | 2 | 3 | calls:1,depth:2 | `draw_border(top-1, left-1, box_w+2, box_h+2);
` |
| 349 | 2 | 3 | calls:1,depth:2 | `draw_table(&vec, sel, col_focus, scroll, top, left, box_w, box_h);
` |
| 350 | 2 | 0 | blank | `` |
| 351 | 2 | 0 | blank | `` |
| 352 | 2 | 3 | calls:1,depth:2 | `refresh();
` |
| 353 | 2 | 0 | blank | `` |
| 354 | 2 | 3 | calls:1,depth:2 | `int ch = getch();
` |
| 355 | 2 | 4 | if,||,depth:2 | `if (ch == 'q' \|\| ch == 'Q') break;
` |
| 356 | 2 | 0 | blank | `` |
| 357 | 2 | 2 | depth:2 | `int rows_area = box_h - 2;
` |
| 358 | 2 | 3 | if,depth:2 | `if (rows_area < 1) rows_area = 1;
` |
| 359 | 2 | 2 | depth:2 | `size_t max_visible = (size_t)rows_area;
` |
| 360 | 2 | 0 | blank | `` |
| 361 | 2 | 3 | switch,depth:2 | `switch (ch) {
` |
| 362 | 3 | 4 | case,depth:3 | `case KEY_UP: case 'k':
` |
| 363 | 3 | 4 | if,--,depth:3 | `if (sel > 0) sel--;
` |
| 364 | 3 | 3 | if,depth:3 | `if (sel < scroll) scroll = sel;
` |
| 365 | 3 | 2 | depth:3 | `break;
` |
| 366 | 3 | 4 | case,depth:3 | `case KEY_DOWN: case 'j':
` |
| 367 | 3 | 3 | if,depth:3 | `if (vec.len == 0) break;
` |
| 368 | 3 | 4 | if,++,depth:3 | `if (sel + 1 < vec.len) sel++;
` |
| 369 | 3 | 3 | if,depth:3 | `if (sel >= scroll + max_visible) scroll = sel - max_visible + 1;
` |
| 370 | 3 | 2 | depth:3 | `break;
` |
| 371 | 3 | 4 | case,depth:3 | `case KEY_LEFT: case 'h':
` |
| 372 | 3 | 4 | if,--,depth:3 | `if (col_focus > 0) col_focus--;
` |
| 373 | 3 | 2 | depth:3 | `break;
` |
| 374 | 3 | 4 | case,depth:3 | `case KEY_RIGHT: case 'l':
` |
| 375 | 3 | 4 | if,++,depth:3 | `if (col_focus + 1 < MAX_COLS) col_focus++;
` |
| 376 | 3 | 2 | depth:3 | `break;
` |
| 377 | 3 | 3 | case,depth:3 | `case 10: // Enter
` |
| 378 | 3 | 3 | case,depth:3 | `case ' ': // Space
` |
| 379 | 3 | 5 | if,&&,calls:1,depth:3 | `if (vec.len > 0 && sel < vec.len) show_details(&vec.data[sel]);
` |
| 380 | 3 | 2 | depth:3 | `break;
` |
| 381 | 3 | 3 | case,depth:3 | `case 'e':
` |
| 382 | 3 | 3 | case,depth:3 | `case 'E':
` |
| 383 | 3 | 4 | if,&&,depth:3 | `if (vec.len > 0 && sel < vec.len) {
` |
| 384 | 4 | 4 | calls:1,depth:4 | `edit_cell(&vec.data[sel], col_focus, top+box_h+1, left);
` |
| 385 | 4 | 3 | depth:4 | `}
` |
| 386 | 3 | 2 | depth:3 | `break;
` |
| 387 | 3 | 3 | case,depth:3 | `case 'a':
` |
| 388 | 3 | 3 | case,depth:3 | `case 'A': {
` |
| 389 | 4 | 3 | depth:4 | `Row r;
` |
| 390 | 4 | 4 | ?:,depth:4 | `r.id = (int)(vec.len ? vec.data[vec.len-1].id + 1 : 1);
` |
| 391 | 4 | 4 | calls:1,depth:4 | `strncpy(r.name, "New Item", sizeof(r.name));
` |
| 392 | 4 | 3 | depth:4 | `r.name[sizeof(r.name)-1]='\0';
` |
| 393 | 4 | 4 | calls:1,depth:4 | `strncpy(r.status, "Pending", sizeof(r.status));
` |
| 394 | 4 | 3 | depth:4 | `r.status[sizeof(r.status)-1]='\0';
` |
| 395 | 4 | 4 | calls:1,depth:4 | `vec_push(&vec, r);
` |
| 396 | 4 | 4 | ?:,depth:4 | `sel = vec.len ? vec.len-1 : 0;
` |
| 397 | 4 | 4 | if,depth:4 | `if (sel >= scroll + max_visible) scroll = sel - max_visible + 1;
` |
| 398 | 4 | 3 | depth:4 | `} break;
` |
| 399 | 3 | 3 | case,depth:3 | `case 'd':
` |
| 400 | 3 | 3 | case,depth:3 | `case 'D':
` |
| 401 | 3 | 4 | if,&&,depth:3 | `if (vec.len > 0 && sel < vec.len) {
` |
| 402 | 4 | 4 | calls:1,depth:4 | `vec_erase(&vec, sel);
` |
| 403 | 4 | 6 | if,&&,--,depth:4 | `if (sel >= vec.len && sel > 0) sel--;
` |
| 404 | 4 | 4 | if,depth:4 | `if (scroll > sel) scroll = sel;
` |
| 405 | 4 | 3 | depth:4 | `}
` |
| 406 | 3 | 2 | depth:3 | `break;
` |
| 407 | 3 | 3 | default,depth:3 | `default:
` |
| 408 | 3 | 0 | comment | `` |
| 409 | 3 | 2 | depth:3 | `break;
` |
| 410 | 3 | 2 | depth:3 | `}
` |
| 411 | 2 | 2 | depth:2 | `}
` |
| 412 | 1 | 0 | blank | `` |
| 413 | 1 | 3 | calls:1,depth:1 | `endwin();
` |
| 414 | 1 | 3 | calls:1,depth:1 | `vec_free(&vec);
` |
| 415 | 1 | 2 | depth:1 | `return 0;
` |
| 416 | 1 | 2 | depth:1 | `}
` |
