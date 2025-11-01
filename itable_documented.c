
---

```c
/******************************************************************************
 * itable_documented.c
 * ---------------------------------------------------------------------------
 * Interactive terminal table built with ncurses.
 * Features:
 *   - Scrollable list of rows (ID, Name, Status)
 *   - Inline editing, adding, deleting
 *   - Status picker and cycler
 *   - CSV export with timestamp
 *   - Memory usage display at startup
 *
 * Compatible with Linux and macOS.
 * Compile:
 *   gcc -std=c99 -Wall -Wextra -O2 -o itable itable_documented.c -lncurses
 ******************************************************************************/

#define _XOPEN_SOURCE 700      // Enables POSIX functions such as getrlimit()

#include <ncurses.h>           // Terminal control library
#include <stdlib.h>            // malloc(), free(), exit()
#include <string.h>            // strcpy(), strncpy(), strlen()
#include <strings.h>           // strcasecmp()
#include <ctype.h>             // isdigit()
#include <sys/resource.h>      // getrlimit(), getrusage()
#include <sys/time.h>          // timeval structure
#include <unistd.h>            // sysconf(), read()
#ifdef __APPLE__
#include <mach/mach.h>         // task_info() for macOS memory stats
#endif
#include <time.h>              // time(), strftime()
#include <errno.h>             // errno codes

/* ---------------------------------------------------------------------------
 * Column layout constants
 * --------------------------------------------------------------------------- */
#define MAX_COLS 3
#define COL0_W 6
#define COL1_W 18
#define COL2_W 12

/* ---------------------------------------------------------------------------
 * Row and vector structures
 * --------------------------------------------------------------------------- */
typedef struct {
    int id;                     // Numeric ID
    char name[64];              // Row name
    char status[32];            // Row status string
} Row;

typedef struct {
    Row* data;                  // Dynamic array of Row
    size_t len;                 // Number of active rows
    size_t cap;                 // Allocated capacity
} RowVec;

/* ---------------------------------------------------------------------------
 * Memory information structure for banner
 * --------------------------------------------------------------------------- */
typedef struct {
    unsigned long long rss_bytes;   // Resident Set Size
    unsigned long long vsize_bytes; // Virtual memory size
    unsigned long long phys_bytes;  // Physical system memory
    unsigned long long lim_as;      // RLIMIT_AS
    unsigned long long lim_data;    // RLIMIT_DATA
    unsigned long long lim_stack;   // RLIMIT_STACK
    int have_proc;                  // 1 if /proc or task_info used
} MemInfo;

/* ---------------------------------------------------------------------------
 * Utility: human-readable byte formatter
 * --------------------------------------------------------------------------- */
static const char* human_bytes(unsigned long long b, char* out, size_t n) {
    const char* units[] = {"B","KB","MB","GB","TB","PB"};
    int u = 0;
    double v = (double)b;
    while (v >= 1024.0 && u < 5) { v /= 1024.0; u++; }
    snprintf(out, n, "%.2f %s", v, units[u]);
    return out;
}

/* Linux helper: read VmRSS or VmSize from /proc/self/status */
#ifdef __linux__
static int read_proc_status_kb(const char* key, unsigned long long* out_kb) {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return 0;
    char line[512];
    int ok = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, key, strlen(key)) == 0) {
            unsigned long long v = 0ULL;
            if (sscanf(line + strlen(key), ":%*s %llu", &v) == 1) {
                *out_kb = v;
                ok = 1;
            }
            break;
        }
    }
    fclose(f);
    return ok;
}
#endif

/* Collect memory info cross-platform */
static void get_mem_info(MemInfo* mi) {
    memset(mi, 0, sizeof(*mi));
    long pages = sysconf(_SC_PHYS_PAGES);
    long pgsize = sysconf(_SC_PAGESIZE);
    if (pages > 0 && pgsize > 0)
        mi->phys_bytes = (unsigned long long)pages * (unsigned long long)pgsize;

    struct rlimit rl;
    if (getrlimit(RLIMIT_AS, &rl) == 0)
        mi->lim_as = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : rl.rlim_cur;
    if (getrlimit(RLIMIT_DATA, &rl) == 0)
        mi->lim_data = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : rl.rlim_cur;
    if (getrlimit(RLIMIT_STACK, &rl) == 0)
        mi->lim_stack = (rl.rlim_cur == RLIM_INFINITY) ? RLIM_INFINITY : rl.rlim_cur;

#ifdef __linux__
    unsigned long long rss_kb=0, vms_kb=0;
    if (read_proc_status_kb("VmRSS", &rss_kb)) mi->rss_bytes = rss_kb * 1024ULL;
    if (read_proc_status_kb("VmSize", &vms_kb)) mi->vsize_bytes = vms_kb * 1024ULL;
    mi->have_proc = 1;
#elif defined(__APPLE__)
    task_basic_info_data_t tinfo;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    kern_return_t kr = task_info(mach_task_self(), TASK_BASIC_INFO,
                                 (task_info_t)&tinfo, &count);
    if (kr == KERN_SUCCESS) {
        mi->rss_bytes = tinfo.resident_size;
        mi->vsize_bytes = tinfo.virtual_size;
        mi->have_proc = 1;
    }
#endif
}

/* Helper: display RLIMIT_* as human-readable */
static void format_limit(char* out, size_t n, unsigned long long lim_bytes) {
    if (lim_bytes == RLIM_INFINITY) snprintf(out, n, "unlimited");
    else human_bytes(lim_bytes, out, n);
}

/* ---------------------------------------------------------------------------
 * Dynamic vector operations
 * --------------------------------------------------------------------------- */
static void vec_init(RowVec* v){ v->data=NULL; v->len=0; v->cap=0; }
static void vec_reserve(RowVec* v,size_t n){
    if(n<=v->cap) return;
    size_t newcap=v->cap? v->cap*2:8;
    if(newcap<n) newcap=n;
    v->data=realloc(v->data,newcap*sizeof(Row));
    v->cap=newcap;
}
static void vec_push(RowVec* v,Row r){ vec_reserve(v,v->len+1); v->data[v->len++]=r; }
static void vec_erase(RowVec* v,size_t i){
    if(i>=v->len) return;
    for(size_t j=i+1;j<v->len;j++) v->data[j-1]=v->data[j];
    v->len--;
}
static void vec_free(RowVec* v){ free(v->data); v->data=NULL; v->len=v->cap=0; }

/* ---------------------------------------------------------------------------
 * Seed example data
 * --------------------------------------------------------------------------- */
static void seed_data(RowVec* v){
    for(int i=1;i<=25;i++){
        Row r;
        r.id=i;
        snprintf(r.name,sizeof(r.name),"Item %02d",i);
        snprintf(r.status,sizeof(r.status),(i%3==0)?"Pending":(i%3==1)?"Active":"Paused");
        vec_push(v,r);
    }
}

/* ---------------------------------------------------------------------------
 * UI helpers: drawing and prompts
 * --------------------------------------------------------------------------- */
static void draw_border(int top,int left,int w,int h){
    mvhline(top,left,0,w);
    mvhline(top+h-1,left,0,w);
    mvvline(top,left,0,h);
    mvvline(top,left+w-1,0,h);
    mvaddch(top,left,ACS_ULCORNER);
    mvaddch(top,left+w-1,ACS_URCORNER);
    mvaddch(top+h-1,left,ACS_LLCORNER);
    mvaddch(top+h-1,left+w-1,ACS_LRCORNER);
}

/* Text input prompt inline */
static int prompt_line_input(int y,int x,const char* prompt,char* buf,int bufsz){
    echo(); curs_set(1);
    mvprintw(y,x,"%s",prompt);
    clrtoeol(); move(y,x+(int)strlen(prompt));
    int res=wgetnstr(stdscr,buf,bufsz-1);
    noecho(); curs_set(0);
    return (res==OK)?0:-1;
}

/* Centered toast message */
static void show_message_center(const char* msg){
    int h,w; getmaxyx(stdscr,h,w);
    int y=h/2,x=(w-(int)strlen(msg))/2;
    attron(A_BOLD); mvprintw(y,x,"%s",msg); attroff(A_BOLD);
    refresh(); getch();
}

/* Detail modal for one row */
static void show_details(const Row* r){
    int h,w; getmaxyx(stdscr,h,w);
    int bw=40,bh=7,top=(h-bh)/2,left=(w-bw)/2;
    draw_border(top,left,bw,bh);
    mvprintw(top+1,left+2,"Row details");
    mvhline(top+2,left+1,ACS_HLINE,bw-2);
    mvprintw(top+3,left+2,"ID: %d",r->id);
    mvprintw(top+4,left+2,"Name: %s",r->name);
    mvprintw(top+5,left+2,"Status: %s",r->status);
    mvprintw(top+bh-1,left+2,"Press any key to return");
    refresh(); getch();
}

/* ---------------------------------------------------------------------------
 * Status management
 * --------------------------------------------------------------------------- */
static const char* STATUS_OPTIONS[]={"Active","Pending","Paused"};
static const int STATUS_COUNT=3;
static int status_index(const char* s){
    for(int i=0;i<STATUS_COUNT;i++)
        if(strcasecmp(s,STATUS_OPTIONS[i])==0) return i;
    return 0;
}
static void set_status(Row* r,int idx){
    if(idx<0||idx>=STATUS_COUNT) return;
    strncpy(r->status,STATUS_OPTIONS[idx],sizeof(r->status));
    r->status[sizeof(r->status)-1]='\0';
}
static void cycle_status(Row* r){
    int i=status_index(r->status);
    i=(i+1)%STATUS_COUNT; set_status(r,i);
}

/* Modal picker for status selection */
static int pick_status_modal(int current){
    int h,w; getmaxyx(stdscr,h,w);
    int bw=22,bh=STATUS_COUNT+4,top=(h-bh)/2,left=(w-bw)/2;
    draw_border(top,left,bw,bh);
    mvprintw(top+1,left+2,"Set Status");
    mvhline(top+2,left+1,ACS_HLINE,bw-2);
    int sel=current;
    while(1){
        for(int i=0;i<STATUS_COUNT;i++){
            int y=top+3+i;
            if(i==sel) attron(A_REVERSE|A_BOLD);
            mvprintw(y,left+2,"%s",STATUS_OPTIONS[i]);
            if(i==sel) attroff(A_REVERSE|A_BOLD);
        }
        mvprintw(top+bh-2,left+2,"Enter: choose  Esc: cancel");
        refresh();
        int ch=getch();
        if(ch==KEY_UP||ch=='k') sel=(sel>0)?sel-1:sel;
        else if(ch==KEY_DOWN||ch=='j') sel=(sel+1<STATUS_COUNT)?sel+1:sel;
        else if(ch==10) return sel;
        else if(ch==27) return -1;
    }
}

/* ---------------------------------------------------------------------------
 * CSV export helpers
 * --------------------------------------------------------------------------- */
static void default_export_path(char* out,size_t n){
    time_t t=time(NULL); struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv,&t);
#else
    localtime_r(&t,&tmv);
#endif
    strftime(out,n,"table_export_%Y%m%d_%H%M%S.csv",&tmv);
}
static void csv_escape(const char* in,char* out,size_t outsz){
    int needs=0;
    for(const char* p=in;*p;p++)
        if(*p==','||*p=='"'||*p=='\n'||*p=='\r'){needs=1;break;}
    if(!needs){snprintf(out,outsz,"%s",in);return;}
    size_t w=0; out[w++]='"';
    for(const char* p=in;*p&&w+2<outsz;p++){
        if(*p=='"'){out[w++]='"'; out[w++]='"';}
        else out[w++]=*p;
    }
    out[w++]='"'; out[w]='\0';
}
static int write_csv(const RowVec* v,const char* path){
    FILE* f=fopen(path,"w"); if(!f) return errno;
    fprintf(f,"ID,Name,Status\n");
    char nbuf[128],sbuf[64];
    for(size_t i=0;i<v->len;i++){
        csv_escape(v->data[i].name,nbuf,sizeof(nbuf));
        csv_escape(v->data[i].status,sbuf,sizeof(sbuf));
        fprintf(f,"%d,%s,%s\n",v->data[i].id,nbuf,sbuf);
    }
    fclose(f); return 0;
}

/* ---------------------------------------------------------------------------
 * Table drawing and editing
 * --------------------------------------------------------------------------- */
static void draw_table(const RowVec* v,size_t sel,size_t col_focus,
                       size_t scroll,int top,int left,int width,int height){
    (void)width;
    attron(A_BOLD|A_UNDERLINE);
    mvprintw(top,left," %-*s %-*s %-*s ",COL0_W,"ID",COL1_W,"Name",COL2_W,"Status");
    attroff(A_BOLD|A_UNDERLINE);
    int visible=height-2;
    for(int i=0;i<visible;i++){
        size_t idx=scroll+i;
        int y=top+1+i;
        if(idx>=v->len) continue;
        const Row* r=&v->data[idx];
        bool selrow=(idx==sel);
        if(selrow) attron(A_REVERSE);
        if(selrow&&col_focus==0) attron(A_BOLD);
        mvprintw(y,left," %-*d ",COL0_W,r->id);
        if(selrow&&col_focus==0) attroff(A_BOLD);
        if(selrow&&col_focus==1) attron(A_BOLD);
        printw(" %-*.*s ",COL1_W,COL1_W,r->name);
        if(selrow&&col_focus==1) attroff(A_BOLD);
        if(selrow&&col_focus==2) attron(A_BOLD);
        printw(" %-*.*s ",COL2_W,COL2_W,r->status);
        if(selrow&&col_focus==2) attroff(A_BOLD);
        if(selrow) attroff(A_REVERSE);
    }
}

/* Edit cell inline */
static void edit_cell(Row* r,size_t col,int footer_y,int left){
    char buf[128];
    switch(col){
        case 0:
            if(prompt_line_input(footer_y,left,"New ID: ",buf,sizeof(buf))==0)
                r->id=atoi(buf);
            break;
        case 1:
            if(prompt_line_input(footer_y,left,"New Name: ",buf,sizeof(buf))==0)
                strncpy(r->name,buf,sizeof(r->name));
            break;
        case 2:
            if(prompt_line_input(footer_y,left,"New Status: ",buf,sizeof(buf))==0)
                strncpy(r->status,buf,sizeof(r->status));
            break;
    }
}

/* ---------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------- */
int main(void){
    RowVec vec; vec_init(&vec); seed_data(&vec);

    if(initscr()==NULL){fprintf(stderr,"ncurses init failed\n");return 1;}
    noecho(); cbreak(); keypad(stdscr,TRUE); curs_set(0);

    /* --- Gather and format memory info --- */
    MemInfo mem; get_mem_info(&mem);
    char rss_h[32],vsz_h[32],phys_h[32],as_h[32],data_h[32],stack_h[32];
    human_bytes(mem.rss_bytes,rss_h,sizeof(rss_h));
    human_bytes(mem.vsize_bytes,vsz_h,sizeof(vsz_h));
    human_bytes(mem.phys_bytes,phys_h,sizeof(phys_h));
    format_limit(as_h,sizeof(as_h),mem.lim_as);
    format_limit(data_h,sizeof(data_h),mem.lim_data);
    format_limit(stack_h,sizeof(stack_h),mem.lim_stack);

    size_t sel=0,col_focus=1,scroll=0;

    while(1){
        erase();
        int h,w; getmaxyx(stdscr,h,w);
        int left=2,header_y=0;

        /* --- Multi-line header with memory info --- */
        mvprintw(header_y++,left,"Interactive Table (rows: %zu)",vec.len);
        mvprintw(header_y++,left,"| Selected Row: %zu",sel);
        mvprintw(header_y++,left,"| Active Column: %zu",col_focus);
        mvprintw(header_y++,left,"| RSS:   %s",rss_h);
        mvprintw(header_y++,left,"| VSZ:   %s",vsz_h);
        mvprintw(header_y++,left,"| Phys:  %s",phys_h);
        mvprintw(header_y++,left,"| AS:    %s",as_h);
        mvprintw(header_y++,left,"| DATA:  %s",data_h);
        mvprintw(header_y++,left,"| STACK: %s",stack_h);
        header_y++;

        int top=header_y+1;
        int box_w=COL0_W+COL1_W+COL2_W+6;
        if(box_w+left+1>w) box_w=w-left-1;
        int box_h=h-top-2;
        if(box_h<6) box_h=6;

        draw_border(top-1,left-1,box_w+2,box_h+2);
        draw_table(&vec,sel,col_focus,scroll,top,left,box_w,box_h);

        int fy=top+box_h-1
	        /* --- Footer help bar ------------------------------------------------- */
	        int fy = top + box_h - 1;          // Compute footer row position
	        move(fy, left);                    // Move cursor to footer row
	        clrtoeol();                        // Clear any leftover text
	        attron(A_DIM);                     // Dimmed text for subtle footer
	        mvprintw(fy, left,
	          " Arrows/kjhl: Move  Enter: View  e: Edit  a: Add  d: Del  s: Status  c: Cycle  x: Export CSV  q: Quit ");
	        attroff(A_DIM);                    // Turn off dim style

	        /* --- Refresh the screen after full draw ------------------------------ */
	        refresh();

	        /* --- Wait for user input --------------------------------------------- */
	        int ch = getch();                  // Read one key press (blocking)

	        /* --- Quit if 'q' or 'Q' pressed -------------------------------------- */
	        if (ch == 'q' || ch == 'Q') break;

	        /* --- Determine how many rows fit on screen --------------------------- */
	        int visible = box_h - 2;
	        if (visible < 1) visible = 1;

	        /* --- Handle all keyboard commands ------------------------------------ */
	        switch (ch) {

	            /* --- Move up ------------------------------------------------------ */
	            case KEY_UP:
	            case 'k':
	                if (sel > 0) sel--;
	                if (sel < scroll) scroll = sel;   // Keep selection visible
	                break;

	            /* --- Move down ---------------------------------------------------- */
	            case KEY_DOWN:
	            case 'j':
	                if (sel + 1 < vec.len) sel++;
	                if (sel >= scroll + (size_t)visible)
	                    scroll = sel - visible + 1;
	                break;

	            /* --- Move left / previous column --------------------------------- */
	            case KEY_LEFT:
	            case 'h':
	                if (col_focus > 0) col_focus--;
	                break;

	            /* --- Move right / next column ------------------------------------ */
	            case KEY_RIGHT:
	            case 'l':
	                if (col_focus + 1 < MAX_COLS) col_focus++;
	                break;

	            /* --- View details modal ------------------------------------------ */
	            case 10:  // Enter
	            case ' ': // Space
	                if (vec.len > 0 && sel < vec.len)
	                    show_details(&vec.data[sel]);
	                break;

	            /* --- Edit current cell ------------------------------------------- */
	            case 'e':
	            case 'E':
	                if (vec.len > 0 && sel < vec.len)
	                    edit_cell(&vec.data[sel], col_focus, top + box_h + 1, left);
	                break;

	            /* --- Add new row -------------------------------------------------- */
	            case 'a':
	            case 'A': {
	                Row r;
	                r.id = (int)(vec.len ? vec.data[vec.len - 1].id + 1 : 1);
	                strncpy(r.name, "New Item", sizeof(r.name));
	                r.name[sizeof(r.name) - 1] = '\0';
	                strncpy(r.status, "Pending", sizeof(r.status));
	                r.status[sizeof(r.status) - 1] = '\0';
	                vec_push(&vec, r);          // Append new row
	                sel = vec.len - 1;          // Move selection to new row
	                if (sel >= scroll + (size_t)visible)
	                    scroll = sel - visible + 1;
	            } break;

	            /* --- Delete current row ------------------------------------------ */
	            case 'd':
	            case 'D':
	                if (vec.len > 0 && sel < vec.len) {
	                    vec_erase(&vec, sel);
	                    if (sel >= vec.len && sel > 0) sel--;
	                    if (scroll > sel) scroll = sel;
	                }
	                break;

	            /* --- Cycle status quickly ---------------------------------------- */
	            case 'c':
	            case 'C':
	                if (vec.len > 0 && sel < vec.len)
	                    cycle_status(&vec.data[sel]);
	                break;

	            /* --- Open status picker modal ------------------------------------ */
	            case 's':
	            case 'S':
	                if (vec.len > 0 && sel < vec.len) {
	                    int cur = status_index(vec.data[sel].status);
	                    int pick = pick_status_modal(cur);
	                    if (pick >= 0)
	                        set_status(&vec.data[sel], pick);
	                }
	                break;

	            /* --- Export to CSV file ------------------------------------------ */
	            case 'x':
	            case 'X': {
	                char path[128];
	                default_export_path(path, sizeof(path));     // Build timestamped filename
	                int rc = write_csv(&vec, path);              // Write rows to file
	                if (rc == 0) {
	                    char msg[256];
	                    snprintf(msg, sizeof(msg), "CSV exported: %s", path);
	                    show_message_center(msg);
	                } else {
	                    char msg[256];
	                    snprintf(msg, sizeof(msg), "CSV export failed (errno %d)", rc);
	                    show_message_center(msg);
	                }
	            } break;

	            /* --- Ignore all other keys --------------------------------------- */
	            default:
	                break;
	        } // end switch
	    } // end main loop

	    /* --- Cleanup before exit ------------------------------------------------- */
	    endwin();          // Restore terminal to normal mode
	    vec_free(&vec);    // Free heap memory used by RowVec
	    return 0;          // Normal exit
	}

	/* ---------------------------------------------------------------------------
	 * End of file
	 * --------------------------------------------------------------------------- */

