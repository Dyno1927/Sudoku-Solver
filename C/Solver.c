// Sudoku Solver in C — my backtracking project while learning C.
// Reads a puzzle from input/puzzle.txt, solves it, prints both out.
// Send 81 digits (0 = empty cell) and it'll backtrack its way to a solution.

#include <stdio.h>

// The board. 0 means empty, 1-9 means filled. Kept global so
// solve(), is_valid() and print_grid() can all read/write the same thing
// without passing a 9x9 array around everywhere.
int grid[9][9];

// Just prints the whole board row by row. Used twice:
// once for the puzzle we got, once for the solved grid.
void print_grid() {
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            // %d, no spaces — numbers just sit next to each other like a real sudoku
            printf("%d", grid[row][col]);
        }
        printf("\n");
    }
}

void write_solution() {
    FILE* output = fopen("output/solution.txt", "w");

    if (output == NULL) {
        perror("Where da output dir gng :/ ?");
        return;
    }

    for (int rows = 0; rows < 9; rows++) {
        for (int cols = 0; cols < 9; cols++) {
            fprintf(output, "%d", grid[rows][cols]);
        }
        fprintf(output, "\n");
    }
    fclose(output);
}

// Checks if placing no_to_try at (row, col) breaks sudoku rules.
// Returns 1 if it fits, 0 if not. Three rules, all basic:
int is_valid(int row, int col, int no_to_try) {
    // Rule 1 & 2: number must not already be in the same row or column.
    // i scans the whole row (grid[row][i]) and whole column (grid[i][col])
    // in one loop. Even the current cell is compared but that's fine —
    // the cell is 0 right now so it never equals the candidate.
    for (int i = 0; i < 9; i++) {
        if (grid[row][i] == no_to_try || grid[i][col] == no_to_try) {
            return 0;  // duplicate found somewhere -> can't place it
        }
    }

    // Rule 3: number must not already be in its 3x3 box.
    // Boxes start at rows/cols 0, 3, 6 — (row / 3) * 3 snaps to the
    // top-left corner of the box the cell lives in.
    int box_row = (row / 3) * 3;
    int box_col = (col / 3) * 3;

    // Sweep the 3x3 box: i/j are offsets (0-2) from that corner.
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[box_row + i][box_col + j] == no_to_try) {
                return 0;  // same number already in the box -> illegal
            }
        }
    }

    // Survived all checks, it's legal to place.
    return 1;
}

// The heart of it: recursive backtracking.
// Returns 1 if a solution is found, 0 if this path dead-ends (so the
// caller knows to undo its move and try a different number).
//
// Idea: find the first empty cell, try 1 to 9 in it. If a number is
// legal, place it and recurse to solve the rest. If the recursion
// comes back saying "nope", undo the move and try the next number.
// The change + undo dance is exactly what "backtracking" means.
int solve() {
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (grid[row][col] == 0) {                         // found a hole to fill
                for (int trying = 1; trying <= 9; trying++) {  // try candidates
                    if (is_valid(row, col, trying)) {
                        grid[row][col] = trying;  // tentatively place it

                        if (solve()) {  // can we finish from here?
                            return 1;   // yes! bubble the "solved" up
                        } else {
                            grid[row][col] = 0;  // no — undo and try next number
                        }
                    }
                }
                return 0;  // none of 1-9 fit this cell -> path is impossible
            }
        }
    }
    return 1;  // no empty cells left -> board is complete
}

int main() {
    // Open the puzzle file. Path is relative to where I run it from,
    // so I run it from inside C/ (gcc Solver.c -o build/Solver && build/Solver).
    FILE* puzzle = fopen("input/puzzle.txt", "r");

    // Don't crash if the file's missing — say what happened and bail.
    if (puzzle == NULL) {
        perror("Where da puzzle man?");
        return 1;  // non-zero exit = "something went wrong" to the shell
    }

    int buf = 0;  // one character read from the file
    int pos = 0;  // how many digits we've stored, 0..80

    // Read characters until the file ends.
    while ((buf = fgetc(puzzle)) != EOF) {
        // buf is a char like '5'. '0' has ASCII value 48, '5' has 53,
        // so '5' - '0' = 53 - 48 = 5. That's the char->number trick.
        // pos / 9 = which row, pos%9 = which column. One flat counter
        // becomes two indexes, no nested loop needed.
        if (buf >= '0' && buf <= '9' && pos < 81) { // ASan global buffer overflow fix as main() reads '\n' too.
            grid[pos / 9][pos % 9] = buf - '0';
            pos++;
        }
    }

    fclose(puzzle);  // always close what I open

    printf("\nIntial Sudoku Puzzle\n");
    print_grid();

    printf("\nSolving...\n\n");

    if (solve()) {
        printf("Solved Sudoku Grid:\n");
        print_grid();
        write_solution();
    } else {
        printf("No solution exists for this Sudoku grid.\n");
    }

    return 0;
}
