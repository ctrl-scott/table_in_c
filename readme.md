

---

````markdown
# 🧮 itable — Interactive Terminal Table (C99 + ncurses)

`itable` is a cross-platform **interactive terminal application** written in C using `ncurses`.  
It displays a scrollable table with editable rows, real-time memory information, and support for exporting to CSV.

The goal of the project is to demonstrate how a text-based user interface can handle structured data — fully offline, in pure C.

---

## ✨ Features

✅ Scrollable table with keyboard navigation  
✅ Inline editing of **ID**, **Name**, and **Status** columns  
✅ **Status management**
  - `s` → opens a modal picker (Active / Pending / Paused)
  - `c` → cycles the status directly  
✅ Add / Delete rows dynamically  
✅ Memory usage banner (RSS, VSZ, limits, total physical)  
✅ Export to **CSV** (press `x`)  
✅ Fully cross-platform (Linux / macOS)  
✅ Lightweight — single C file, ~800 lines with comments

---

## 🧱 Build Instructions

### Linux (Debian / Ubuntu)

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
gcc -std=c99 -Wall -Wextra -O2 -o itable itable.c -lncurses
````

### macOS (Homebrew)

```bash
brew install gcc ncurses
gcc -std=c99 -Wall -Wextra -O2 -o itable itable.c \
  -I"$(brew --prefix)/opt/ncurses/include" \
  -L"$(brew --prefix)/opt/ncurses/lib" -lncurses
```

### Optional Makefile

```make
CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -O2
# macOS only:
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

## ▶️ Run

```bash
./itable
```

---

## ⌨️ Controls

| Key               | Action                                         |
| ----------------- | ---------------------------------------------- |
| ↑ / ↓  or  k / j  | Move selection                                 |
| ← / →  or  h / l  | Change active column                           |
| `Enter` / `Space` | View details of selected row                   |
| `e`               | Edit active cell (ID, Name, Status)            |
| `a`               | Add new row                                    |
| `d`               | Delete selected row                            |
| `s`               | Open status picker (Active / Pending / Paused) |
| `c`               | Cycle status quickly                           |
| `x`               | Export current table to CSV                    |
| `q`               | Quit                                           |

---

## 🧾 CSV Export

Press **`x`** to export the current table to a CSV file in your working directory.

Example output filename:

```
table_export_20251101_170250.csv
```

Example content:

```csv
ID,Name,Status
1,Item 01,Active
2,Item 02,Paused
3,Item 03,Pending
```

Fields are properly CSV-escaped (quotes doubled, quoted if needed).

---

## 🧠 Memory Banner

On startup, `itable` prints memory stats that remain visible at the top of the interface:

| Field | Meaning                              |
| ----- | ------------------------------------ |
| RSS   | Resident memory used by this process |
| VSZ   | Virtual memory allocated             |
| Phys  | Total physical RAM (system)          |
| AS    | Address-space limit (`RLIMIT_AS`)    |
| DATA  | Data-segment limit (`RLIMIT_DATA`)   |
| STACK | Stack limit (`RLIMIT_STACK`)         |

* **Linux:** via `/proc/self/status` and `getrusage`
* **macOS:** via `task_info()` and `getrusage`

---

## 🖼️ Screenshots (optional)

You can include screenshots here after running:

```bash
gnome-terminal -- ./itable
```

Example:

```
+--------------------------------------------+
| Interactive Table (rows: 25)               |
| RSS: 3.2 MB  VSZ: 21.5 MB  Phys: 8 GB      |
| ...                                        |
+--------------------------------------------+
| ID | Name             | Status             |
|  1 | Item 01          | Active             |
|  2 | Item 02          | Paused             |
|  3 | Item 03          | Pending            |
+--------------------------------------------+
```

---

## 🧩 Code Overview

| Function            | Description                                      |
| ------------------- | ------------------------------------------------ |
| `get_mem_info`      | Retrieves system memory and process stats        |
| `draw_table`        | Draws the visible portion of the table           |
| `edit_cell`         | Inline edit of ID/Name/Status                    |
| `pick_status_modal` | Popup menu for changing status                   |
| `cycle_status`      | Quickly rotates Active→Pending→Paused            |
| `write_csv`         | Writes the current table to disk                 |
| `vec_*`             | Small dynamic array (push, erase, reserve, free) |

---

## 📚 Educational Notes

This program demonstrates:

* Building TUI (Text User Interfaces) with **ncurses**
* Cross-platform resource inspection in C
* Implementing modal dialogs and dynamic UI state
* Safe manual memory management and dynamic arrays
* Writing structured data (CSV) safely with escaping
* Handling interactive keyboard input (`getch()` loop)

---

## 🧠 Future Enhancements

* Color-coded statuses (Active = green, Paused = yellow, Pending = red)
* Load data from CSV at startup
* Sorting and filtering by column
* Mouse input support
* Persistent configuration file

---

## ⚖️ License

This project is released under an open educational license.
You are free to use, modify, and distribute for **educational or non-malicious** purposes.

Attribution appreciated
---

## 💬 Credits

Developed collaboratively with assistance from OpenAI GPT-5.
Runs natively on Debian 13 and macOS 14 with GCC 13 + ncurses 6.

---

## 🧭 Repository Structure

```
itable/
├── itable.c                # Main application source
├── itable_documented.c     # Fully commented version
├── README.md               # This documentation
├── Makefile                # Optional build script
└── table_export_*.csv      # Example export outputs
```

---

> *“Everything you can do with a GUI, you can also do with a TUI — if you treat the terminal like a canvas.”*

```

---
