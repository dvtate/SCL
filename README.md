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
- [x] `-r` : run compressed bytecode
- [x] `-h` : print help msg
- [ ] No args : run REPL

### Compile
#### Bytecode Text
Useful for debugging compiler. Also prints compile errors.
```
[build] $ echo 'print("Hi")' > test.s && ./dlang -ftest.s -o
# Literal 0:
String: "Hi"
# Literal 1:
Macro: (:
        LET_ID 10
        LET_ID 11
        USE_LIT 0
        USE_ID 1
        INVOKE
)
#### Begin Fault Table ####
ID_NAME o : ID_ID 11
ID_NAME print : ID_ID 0
ID_NAME i : ID_ID 10
ID_NAME input : ID_ID 1
In file: test.s
Compiled Line#3 came from Source Pos#6
Compiled Line#4 came from Source Pos#0
```

#### Bytecode bin
The following command will compile  `print("Hi")` in test.s into binary test.bin. Run until it doesn't give syntax errors before sending output to test.bin
```
[build] $ echo 'print("Hi")' > test.s && ./dlang -ftest.s -o > test.bin
```


# Basic Syntax
Note these are
- 1. Mostly not implemented yet: building things in phases starting with basic functionality
- 2. Not set in stone: if you have an opinion, send a PR and we can make a better language together.

## Language Structure
- Statements end with semicolons (`;`)
    - ASI exists but untrustworthy
- Not whitespace dependent

## Comments
These get ignored (maybe switch to `#` ?)
- `//` line comments
- `/* ... */` multi-line comments

## Variables/References

### Builtin Global Variables
These are likely to change. All of these values are reassignable and can be referenced and called within other scopes. 
- `print`: write values to terminal (ie: `print("Hello, world!")`)
- `input`: read values from terminal
- `i`: command line arguments (Note: only at global scope, see macros section)
- `o`: leave current scope with return value provided as argument (ie: `o(-1)`)

### Declaration
You can declare a variable using the let operator. 
```
let name = "John Smith";
let age = 30, vehicle;
vehicle = "Hot rod"; 
```

### Referencing
It's important to note that all variables are actually references, (more on this later). To copy a variable by value instead of by reference use `$`/`copy`. To create a reference for a value use `ref`.

## Values
Supports any valid JSON data. Note there are a number of functions 
- [x] Strings `"hello"`
- [x] Ints `10`
- [x] Floats `1.2`
- [ ] objects/dicts `{ "temp": 98.6, }`
- [ ] lists/arrays `[1, 2.5, "cat" ]`

## Macros/Lambdas
Macros/Lambdas are like first-class functions except they only have one input and one output.

### Making a Macro
- Macro literals are enclosed in `(:` `)`
- Variables can reference macros just like any other data, however code cannot modify their internals
```
let name = input();
let say_hello = (:
    print("hello, " + input);
);
say_hello(); // greets user
```

### Input and Output
Input is accessible via the local variable i. Use the local variable o to return a value. Although you can use i and o themselves, declaring variables for them can improve clarity.
```
let greeting = (:
    let name = i;
    let return = o;
    return("Hello, " + name);
);
print(greeting(input()))
```

## Control Flow
These are currently defined as builtin globals, but in the future they will likely be converted to proper operators. 

### Conditionals
Traditional If-else style conditionals. Note: the else argument is optional (also note comma separated arguments converted to a list)

```
let gpa = Number(input());

if (gpa > 4 || gpa < 0, (:
    print("seems rigged");
), (:
    if (gpa >= 2, (:
        print("PASS");
    ), (:
        print("FAIL");
    ));
));
```

### Looping
#### While Loops
Pretty standard apart from it not being an operator.
```
let n = 0;
while ((: n < 5 ), (: 
    n += 1;
    print(n);
);
```

#### Range Based For
Given increased user tools for control flow (no confusion with return/await/etc. being operators), I don't think this language needs a C-Style For loop. As an alternative Collections have their own implementations of for_each, map, filter, etc. as members.

```
range(0, 5).for_each((:
    print(i); // 012345
));
```

## More coming soon
Some of these features already aren't in this rough draft of software, 
