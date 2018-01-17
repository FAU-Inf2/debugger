# Debugger

A sample implementation of a very basic debugger for x86-64/Linux.

## Compile

Type `make` to compile the debugger. This should generate the executable `bin/debugger`.

## Run

To run the debugger, simply type `./bin/debugger <breakpoint address> <debugee command line>`.

For example, type `./bin/debugger 0x4000ed ./debugees/fib`. The debugee program computes the 5th
Fibonacci number in the register `%rax`, as can be observed by the debugger output.

## License

This program is licensed under the terms of the MIT license, see `LICENSE.txt`.
