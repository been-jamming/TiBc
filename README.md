# TiBc
TiBc is a native compiler on the Ti89 for a B-like programming language currently in development. 

### To compile
To compile with GCC, simply use
```
make
````
To compile with any other compiler, my_compiler for instance, use
```
make CC=my_compiler
```
These output an executable `TiBc`

### To use
#### Unix
Use one of the following
```
./TiBc input_file.b
```
Or if you would like to specify an output file
```
./TiBc input_file.b output_file.out
```
#### Windows
Use one of the following
```
TiBc input_file.b
```
Or if you would like to specify an output file
```
TiBc input_file.b output_file.out
```

### The Language
The language TiBc compiles currently is an extremely simple and B-like.
#### Keywords
* `var`
  * Declares a variable or function. It cannot initialize a variable with a value.
  * Variables can only contain integer values of a predefined width - no other data type is supported or needed.
  * Example: `var my_var; var my_function(arg1, arg2){...`
* `if`
  * Gives a condition for the execution of a code block.
  * Example: `if(my_var == 2){my_var -= 1;...}`
* `while`
  * Gives a condition for the repeated execution of a code block.
  * Example: `while(val > 1){output = output*val; val = val - 1}`
* `return`
  * Specifies the output of a function
  * Example: `var my_function(a, b){var output; ... return output;}`
#### Currently Supported Operators
* Arithmetic operators:```+ - * /```
* Logical operators:```& |```
  * if you desperately need the not `!` operator, you can implement it as a function. This will be added ASAP.
* Comparison operators:```< > ==```
* Referencing operators:```* &```

The language specifies standard C order of operations except for the fact that a value/variable is always referenced/dereferenced before it is executed as a function. For example
```
*my_var(a, b, c)
```
is the same as
```
(*my_var)(a, b, c)
```
This will most likely be changed in future versions.
#### Example Code
A few examples are in the `Examples` folder

*factorial*
```
var factorial(n){
  if(n < 2){
    return 1;
  }
  
  return n*factorial(n - 1);
}
```

### Output
TiBc currently outputs pseudo-assembly for a stack machine.

Places on the stack are referenced relative to the current location of the stack pointer. Positive references are where the stack is going, and negative references on the stack refer to previous values pushed onto the stack. The references assume that the stack pointer is incremented first before writing the value when pushing to the stack. This, however, can be easily changed for a different architecture.

Stack relative references are with respect to the width of the architectures native word (or possibly a different width). On the Ti89, each integer will be 

*opcodes*
* PUSH
  * Pushes an individual value onto the stack, either a constant or relative to the stack pointer.
* POP
  * Pops an individual value from the top of thte stack, storing it into either a global address, indirect address on the stack, or into a value on the stack
* MOV
  * Moves a value between stack or global addresses.
* BZSTACK
  * Pops a value from the stack and branches to the address given if that value was 0
* BNZSTACK
  * Pops a value from the stack and branches to the address given if that value was not 0
* SSP
  * Changes the stack pointer directly by n addresses. If positive, make the stack bigger. If negative, make it smaller by that number.
* ADDSTACK
  * Pops the last two values on the stack and pushes their sum.
* SUBSTACK
  * Pops the last two values on the stack and pushes the second subtracted from the first.
* MULSTACK
  * Pops the last two values on the stack and pushes their product.
* DIVSTACK
  * Pops the last two values on the stack and pushes the first divided by the second.
* ORSTACK
  * Pops the last two values on the stack and pushes the result of a bitwise or between the two.
* ANDSTACK
  * Pops the last two values on the stack and pushes the result of a bitwise and between the two.
* LTSTACK
  * Pops the last two values on the stack and pushes `1` if the first was less than the second, otherwise it pushes `0`
* GTSTACK
  * Pops the last two values on the stack and pushes `1` if the first was greater than the second, otherwise it pushes `0`
* EQSTACK
  * Pops the last two values on the stack and pushes `1` if the first was equal to the second, otherwise it pushes `0`
* JMPSTACK
  * Pops the last value on the stack and jumps th that address.
  * Program labels, memory addresses, and integers are all expected to have the same bit-width.
* DEREFSTACK
  * Pops the last value on the stack and pushes the value in memory stored at the address popped.
* CONSTANT
  * Defines an in-place constant in memory

### To Do
* Implement the '!' operation (honestly should have already been done)
* Simple optimization
* Allocate variables to registers
* Output \_nostub executables which run on Ti89 and Ti92 calculators
* Get the compiler to run on the calculator itself with a text editor to make it a native compiler.
