# dlang20
New language I'm working on. 

## Build
```
$ mkdir build
$ cd build
$ cmake ..
$ make -j `nproc`
```

## Run
Currently more focused on language stuff than main program, sorry this is ugly for now (feel free to send pr). Notice the separated compile and run operations.
- [x] `-f` : required input file
- [x] `-O` : compile and output bytecode text
- [x] `-o` : compile and output compressed bytecode
- [ ] `-r` : run compressed bytecode
- [ ] `-h` : print help msg
- [ ] No args : run REPL

# Basic Syntax

## Language Structure
- Basic Syntax is largely inspired by JavaScript as it's the most popular language currently.
- Statements end with semicolons (`;`)
- Not whitespace dependent

## Comments
These get ignored 
- `//` line comments
- `/* ... */` multi-line comments

## Variables/References

### Declaration
You can declare a variable using the let operator. 
```
let name = "John Smith";
let age = 30, vehicle;
vehicle = "Hot rod"; 
```
### Referencing
It's important to note that all variables are actually references, (more on this later). To copy a variable by value instead of by reference use the `$` operator.
```
// make a reference 5
let a = 5;
// make b reference a
let b = a;

// make a reference 6
a = 6;
print(b); // 6

// change the value referenced by b to 10
b =: 10;
print(a); // 10

// make c reference a copy of b
let c = $b;
```

## Values
Supports JSON data. 
- [x] Strings `"hello"`
- [x] Ints `10`
- [x] Floats `1.2`
- [ ] objects/dicts `{}`
- [ ] lists/arrays `[]`

## Macros/Lambdas
Macros/Lambdas are like first-class functions except they only have one input and one output.
- WiP...

## More coming soon
