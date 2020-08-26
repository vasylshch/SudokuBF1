# SudokuBF1

Brute force Sudoku solver

## Instructions
Open Visual Studio solution or use cmake:

```sh
cmake --build build
build/SudokuBF1 003020600900305001001806400008102900700000008006708200002609500800203009005010300
echo $?
# on Windows: echo %ERRORLEVEL%
# To build Release: -DCMAKE_BUILD_TYPE=Release
```

## Motivation
It should be:
* short
* clear
* performance does matter
* 1 file
* brute force algo on finding all sudoku's solutions close to current one
* sss_size - configurable compile time constant
