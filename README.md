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
$ echo 'print("Hi")' > test.s && ./dlang -ftest.s -o
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
The following command will compile  `print("Hi")` in test.s. If it doesn't have any syntax errors it will output a file `o.bin` which you can run with the `-r` flag
```
$ echo 'print("Hi")' > test.s && ./dlang -ftest.s -o
compiled to o.bin
$ ./dlang -fo.bin -r
Hi
$
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
|Ready|Type|Literal| Use
|---|---|---|---|
|<ul><li>[x] </li></ul>|`Str`|`"Hello, world!"`| Holds character sequences|
|<ul><li>[x] </li></ul>|`Int`|`10`| Holds whole numbers (64 bits) |
|<ul><li>[x] </li></ul>|`Float`|`1.2`| 64bit floating point numbers |
|<ul><li>[x] </li></ul>|`a -> b`|`(: i + 2 )` | first-class functions, alternatives to blocks|
|<ul><li>[x] </li></ul>|`List`|`[1, 2.5, 'cat']`| Hold series of values |
|<ul><li>[ ] </li></ul>|`Obj`|`{ temp: 98.6 }`| Holds associative |

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
Input is accessible via the local variable i. Use the local variable o to return a value. Although you can use i and o themselves, declaring variables (`let`) or aliases (`using`) for them can improve clarity and is required if they get shadowed by a previous scope.
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
));
```
Also note that you can implement `while` on your own like this:
```
let while = (:
    using args = i, break = o;
    if (args[0](break), (:
        args[1](break);
        while(args);
    ));
);
```
- Calling `i()` from body or condition will break out of the loop
- Calling `o()` from the body will skip to next cycle
- Calling `i(true)` will make `while(...)` return `true` when you break out

#### Range Based For
Given increased user tools for control flow (no confusion with return/await/break/continue/etc. being operators), I don't think this language needs a C-Style For loop. As an alternative Collections have their own implementations of for_each, map, filter, etc. as members.

```
range(0, 5).for_each((:
    print(i); // 012345
));
```

## More coming soon
Some of these features already aren't in this rough draft of software, 
