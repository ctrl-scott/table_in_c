# 🧮 itable — Interactive Terminal Table (C99 + ncurses)

`itable` is a cross-platform **interactive terminal application** written in C using `ncurses`.  
It displays a scrollable table with editable rows, real-time memory information, and support for exporting to CSV.

The goal of the project is to demonstrate how a text-based user interface can handle structured data — fully offline, in pure C. Further, the goal of the project is to take C and say that it is still very relevant and very useful in many ways especially in demostration and time constraint.

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
