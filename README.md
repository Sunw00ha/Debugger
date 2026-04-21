# nuPython debugger

An interactive debugger for **nuPython** programs: step execution, breakpoints, inspect variables and memory—same general idea as gdb, for this small interpreted language.

The interpreter (parser, execution engine, etc.) ships as a prebuilt `nupython.o`; this repo adds the debugger on top (`debugger.cpp` / `debugger.h`) plus a small `main.cpp` driver. Sample scripts are `test*.py` in the project folder.

## How to run it

cd "source level debugger for nuPython"
make build
./a.out path/to/script.py


To type a program by hand instead of using a file:
./a.out

Paste or type your nuPython source, then enter `$` alone on a line to finish.

Inside the debugger, press `h` for help. 

Shortcuts: `r` to run or continue, `s` to step one line, `b n` / `rb n` to set or remove a breakpoint on line `n`, `p name` to print a variable, `q` to quit.

`make clean` removes the built binary. For leak checking, `make valgrind` runs the program under Valgrind.
