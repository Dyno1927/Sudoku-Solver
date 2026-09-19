# Sudoku Solver (C)

My backtracking sudoku solver written in C while learning the language.
Reads a puzzle from a text file, solves it with backtracking, prints both.

## Project layout

```
Sudoku-Solver/
└── C/
    ├── Solver.c           # the whole program
    └── input/
        └── puzzle.txt     # the puzzle to solve
```

## How it works

1. **Read** — `main()` opens `input/puzzle.txt` and stores each digit into
   the 9x9 `grid`. A flat position counter `pos` (0–80) maps to the grid
   with `pos/9` (row) and `pos%9` (column). `0` = empty cell.
2. **Check** — `is_valid()` enforces the three sudoku rules: no duplicate
   in the row, column, or 3x3 box.
3. **Solve** — `solve()` finds the first empty cell, tries digits 1–9,
   recurses, and **un-does the move** if that path dead-ends. That
   change-and-undo loop is backtracking.
4. **Print** — `print_grid()` shows the puzzle and the solution.

## Building & running

```sh
mkdir -p build
gcc Solver.c -o build/Solver && build/Solver
```

Run from inside `C/` so the relative path `input/puzzle.txt` resolves.

For warnings during development:
```sh
gcc -Wall -Wextra -g -O0 Solver.c -o build/Solver && build/Solver
```

## Puzzle format

Exactly 81 digits, or a grid of 9 lines x 9 digits, `0` for empty cells.
Whitespace is fine — the reader just skips non-digits. Example
(Wikipedia's "Sudoku solving algorithms" puzzle):

```
530070000
600195000
098000060
800060003
400803001
700020006
060000280
000419005
000080079
```