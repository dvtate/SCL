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
Note these are
- 1. Mostly not implemented yet: building things in phases starting with basic functionality
- 2. Not set in stone: if you have an opinion, send a PR and we can make a better language together.

## Language Structure
- Basic Syntax is largely inspired by JavaScript as it's the most popular language currently.
- Statements end with semicolons (`;`)
    - In many cases they are optional, but it's best to just 
- Not whitespace dependent

## Comments
These get ignored 
- `//` line comments
- `/* ... */` multi-line comments

## Variables/References

### Builtin Glboal Variables
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
