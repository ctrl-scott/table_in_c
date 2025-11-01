Here is a **paste-ready README** for `itable.c` and all the additions we implemented (memory banner, multi-line header, status picker/cycler, CSV export). You can save this as `README.md` in the same folder as `itable.c`.

---

# itable — Interactive Terminal Table (ncurses, C99)

`itable.c` is a small, portable terminal program that renders a scrollable table and lets you interact with each row. It uses **ncurses** and runs on Linux and macOS.

## Features

* Scrollable table with selection and per-row actions
* Inline cell editing (ID, Name, Status)
* **Status updates**:

  * `s` opens a picker modal: **Active / Pending / Paused**
  * `c` cycles the status quickly
* **CSV export**: press `x` to write `table_export_YYYYMMDD_HHMMSS.csv`
* **Memory banner** at startup:

  * Process RSS, virtual size, system physical RAM
  * Resource limits: RLIMIT_AS, RLIMIT_DATA, RLIMIT_STACK
* **Multi-line header** support (no longer overwritten by the table)

---

## Build

### Linux (Debian/Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
gcc -std=c99 -Wall -Wextra -O2 -o itable itable.c -lncurses
```

### macOS (Homebrew)

```bash
brew install gcc ncurses
gcc -std=c99 -Wall -Wextra -O2 -o itable itable.c \
  -I"$(brew --prefix)/opt/ncurses/include" \
  -L"$(brew --prefix)/opt/ncurses/lib" -lncurses
```

### Optional Makefile (drop-in)

```make
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -O2
# Uncomment the next two lines on macOS if needed:
# CFLAGS += -I$(shell brew --prefix)/opt/ncurses/include
# LDFLAGS += -L$(shell brew --prefix)/opt/ncurses/lib
LDLIBS=-lncurses

all: itable
itable: itable.c
run: itable
	./itable
clean:
	rm -f itable
```

---

## Run

```bash
./itable
```

---

## Controls

| Key                | Action                                            |
| ------------------ | ------------------------------------------------- |
| ↑ / ↓ or `k` / `j` | Move selection                                    |
| ← / → or `h` / `l` | Change active column                              |
| `Enter` / `Space`  | View details for selected row                     |
| `e`                | Edit the active cell (ID, Name, or Status)        |
| `a`                | Add a new row                                     |
| `d`                | Delete selected row                               |
| `s`                | **Status picker** modal (Active, Pending, Paused) |
| `c`                | **Cycle** status quickly                          |
| `x`                | **Export CSV** of the table to a timestamped file |
| `q`                | Quit                                              |

---

## CSV Export

* Trigger: press **`x`**.
* Output: `table_export_YYYYMMDD_HHMMSS.csv` in the working directory.
* Columns: `ID,Name,Status`.
* Fields are properly CSV-escaped (quotes doubled, quoted if needed).

**Example**

```csv
ID,Name,Status
1,Item 01,Active
2,Item 02,Paused
3,"Complex, Name",Pending
```

---

## Memory Banner

Displayed at the top when the application starts (and kept visible):

* **RSS** (resident set size) and **VSZ** (virtual size)

  * Linux: `/proc/self/status` (VmRSS, VmSize) with fallback to `getrusage`
  * macOS: `task_info(TASK_BASIC_INFO)` with fallback to `getrusage`
* **Phys**: total physical memory from `sysconf(_SC_PHYS_PAGES)`
* **Limits**: RLIMIT_AS, RLIMIT_DATA, RLIMIT_STACK (shows `unlimited` when applicable)

If the terminal is narrow, you may split the header into multiple lines (already supported) or reduce which fields are shown.

---

## Source Layout (key functions)

* **Data structures**

  * `typedef struct Row { int id; char name[64]; char status[32]; }`
  * `typedef struct RowVec { Row* data; size_t len; size_t cap; }`

* **Vector helpers**

  * `vec_init`, `vec_reserve`, `vec_push`, `vec_erase`, `vec_free`

* **UI and drawing**

  * `draw_border(...)` — simple box border
  * `draw_table(...)` — renders header row and visible rows
  * `show_details(const Row*)` — centered modal with row details
  * `prompt_line_input(...)` — inline input prompt for editing

* **Memory reporting**

  * `get_mem_info(MemInfo*)`, `human_bytes(...)`, `format_limit(...)`
  * Linux helper: `read_proc_status_kb(...)`
  * macOS: `task_info(...)` branch

* **Status management**

  * `STATUS_OPTIONS[] = {"Active","Pending","Paused"}`
  * `status_index`, `set_status`, `cycle_status`
  * `pick_status_modal(int current_idx)` — arrow-key modal picker

* **CSV export**

  * `default_export_path(char*, size_t)` — builds timestamped filename
  * `csv_escape(const char*, char*, size_t)` — minimal CSV escaping
  * `write_csv(const RowVec*, const char* path)` — writes header + rows

---

## Implementation Notes

* **Header height awareness**: the render loop computes a `header_y` value as lines are printed, then places the table box below it. This prevents the table from overwriting the header when the header spans multiple lines.
* **Editing prompts** are shown below the table box to avoid overlapping the table area.
* **Status modal** uses reverse and bold attributes for the active item and accepts `↑/↓/k/j`, `Enter`, and `Esc`.
* **Warnings**: earlier unused parameters can be either removed or silenced using `(void)param;` statements. If you prefer a warning-free build without code changes, use `-Wno-unused-parameter -Wno-unused-function`.

---

## Troubleshooting

* **`ncurses` not found (macOS):** add Homebrew include and lib flags as shown above.
* **Broken characters/box drawing:** ensure `LANG` and terminal encoding support Unicode, or replace line drawing with ASCII characters.
* **Tiny terminals:** reduce column widths or break the header into more lines. The table will still function.

---

## Extending

* **Colors per status:** define color pairs and apply based on `row->status`.
* **Sorting:** add a sort key (toggle with a key) and sort `RowVec` in place.
* **Persistence:** load/save JSON or CSV between runs.
* **Mouse support:** enable mouse in ncurses and map clicks to selection and actions.
* **Filtering:** add a quick filter prompt to show rows matching a substring.

---

## License

Use freely for educational and non-malicious purposes. Attribution appreciated but not required.

---

If you want, I can generate this README with a **diagram of the render layout** and an **annotated screenshot** section, or add a **man page** (`itable.1`) formatted for `man`.
