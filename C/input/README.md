# input

Where `puzzle.txt` lives. The pipeline drops a fresh websudoku puzzle here
every run, so it's gitignored — committing it would churn random digits on
every solve.

To feed the solver a puzzle by hand, I drop any 81-digit string here
(`0` = empty cell) and run it from `C/`:
`gcc -Wall -Wextra Solver.c -o build/Solver && build/Solver`