Perfect — here’s a detailed **ASCII layout diagram** showing how the `itable` ncurses interface is structured in memory and on screen, including the header, table, and footer sections, plus notes on the key drawing order and refresh cycle.

---

# 🧭 `itable` Screen Layout (ASCII Diagram)

```
┌───────────────────────────────────────────────────────────────────────────┐
│  Interactive Table (rows: 25)                                             │
│  | Selected Row: 3                                                        │
│  | Active Column: 2                                                       │
│  | RSS:   3.25 MB                                                        │
│  | VSZ:   21.45 MB                                                       │
│  | Phys:  8.00 GB                                                        │
│  | AS:    unlimited                                                      │
│  | DATA:  unlimited                                                      │
│  | STACK: 8.00 MB                                                        │
│                                                                           │
│───────────────────────────────────────────────────────────────────────────│
│ ┌───────────────────────────────────────────────────────────────────────┐ │
│ │  ID    Name                Status                                     │ │
│ │ ───────────────────────────────────────────────────────────────────── │ │
│ │   1    Item 01             Active                                    │ │
│ │   2    Item 02             Paused                                    │ │
│ │   3    Item 03             Pending                                   │ │
│ │   4    Item 04             Active                                    │ │
│ │   5    Item 05             Paused                                    │ │
│ │   6    Item 06             Pending                                   │ │
│ │   ...                                                              ...│ │
│ │   ↑/↓ scrolls visible region; highlighted row = current selection      │ │
│ └───────────────────────────────────────────────────────────────────────┘ │
│───────────────────────────────────────────────────────────────────────────│
│  Arrows/kjhl: Move  Enter: View  e: Edit  a: Add  d: Del                 │
│  s: Status  c: Cycle  x: Export CSV  q: Quit                             │
└───────────────────────────────────────────────────────────────────────────┘
```

---

## 🧩 Visual Breakdown by Region

| Region                     | Location                             | Function                                                                               |
| -------------------------- | ------------------------------------ | -------------------------------------------------------------------------------------- |
| **Header / Memory Banner** | Top of window (`header_y` rows tall) | Displays live memory info (RSS, VSZ, Phys, limits) and selected row metadata.          |
| **Border Box**             | Below header                         | Drawn via `draw_border()` with `ACS_*` line characters; encloses the scrollable table. |
| **Table Header Row**       | Inside the box (`top` row)           | Column labels ("ID", "Name", "Status"), bold + underlined.                             |
| **Table Body**             | Inside box, below header row         | Visible subset of rows from `RowVec` (`scroll` controls offset).                       |
| **Footer Help Line**       | Immediately below box                | Shows key mappings; dimmed for lower visual priority.                                  |

---

## 🧱 Internal Memory Layout (Conceptual)

```
ncurses stdscr buffer
┌────────────────────────────────────────────────────────────┐
│ [header lines: printed sequentially, header_y tracks count] │
│ [table box top/bottom lines]                               │
│ [table rows: each mvprintw at (top + i, left)]              │
│ [footer line: printed at fy = top + box_h - 1]              │
└────────────────────────────────────────────────────────────┘
```

### Rendering Order per Loop

1. **erase()** — clears entire screen buffer
2. **draw header** — multi-line memory + status info (`mvprintw` repeatedly, incrementing `header_y`)
3. **draw_border()** — box outline for table
4. **draw_table()** — prints table header + visible rows
5. **footer help line** — drawn at `fy` after table
6. **refresh()** — pushes all changes to terminal display

This order ensures the header never gets overwritten by the table even when the terminal resizes or the header has multiple lines.

---

## 🎨 Focus and Selection

```
[Normal Row]    plain text
[Selected Row]  reverse video (A_REVERSE)
[Focused Column] bold text (A_BOLD)
```

When you move horizontally between columns, the focused column is bolded within the reversed row — visually separating the active edit target.

---

## 🧮 Status Modal Example (when pressing `s`)

```
+----------------------+
| Set Status           |
|----------------------|
| Active               |
| → Pending            |
|   Paused             |
|----------------------|
| Enter: choose  Esc: cancel |
+----------------------+
```

* `A_REVERSE | A_BOLD` highlights the selected item.
* Arrow keys (`↑` / `↓`) navigate; Enter selects, Esc cancels.
* Centered automatically using `getmaxyx()` window dimensions.

---

## 📦 CSV Export Flow (triggered by `x`)

```
User presses 'x'
 ├─► default_export_path() → table_export_YYYYMMDD_HHMMSS.csv
 ├─► write_csv(&vec, path)
 │     ├─ writes header: "ID,Name,Status"
 │     └─ loops through all rows, CSV-escapes fields
 ├─► show_message_center("CSV exported: ...")
 └─► File written in current working directory
```

---

## 🔁 Event Flow (Main Loop Overview)

```
┌──────────────────────────────┐
│ while (1) {                 │
│   erase();                  │
│   draw header;              │
│   draw border + table;      │
│   draw footer;              │
│   refresh();                │
│   ch = getch();             │
│   switch(ch) {              │
│     ... handle key input ...│
│   }                         │
│ }                           │
└──────────────────────────────┘
```

The **main loop** redraws the screen completely every iteration, making layout deterministic and easy to manage.

---

## 🧠 Conceptual OSI-like Layering (UI Layers)

```
+---------------------------------------------------+
| Application Logic (RowVec, CSV, Memory Stats)     |
+---------------------------------------------------+
| UI Rendering Layer (draw_table, draw_border)      |
+---------------------------------------------------+
| Input/Event Loop (getch, switch handlers)         |
+---------------------------------------------------+
| ncurses Rendering Engine (refresh buffer)         |
+---------------------------------------------------+
| Terminal Emulator (VT/ANSI)                       |
+---------------------------------------------------+
```

Each layer is isolated — logic doesn’t directly manipulate the screen buffer except through `mvprintw` and `ncurses` functions.

---

## 💡 Design Principles

1. **Stateless drawing** — each frame completely repaints UI for clarity.
2. **Platform-safe memory** — uses only standard libc and ncurses.
3. **Responsive scaling** — header height automatically shifts table position.
4. **Minimal global state** — everything is stack-local except the main vector.
5. **Educational readability** — designed to demonstrate TUI structure, not optimization tricks.

---

Would you like me to follow this with a **memory map ASCII diagram** (showing heap/vector allocations, stack, and ncurses buffers) — similar to an internal architectural “view under the hood”? It complements this screen layout for teaching or documentation purposes.
