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
$ echo 'print("Hi")' > test.s && ./dlang -ftest.s -O
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
- 1. Some features aren't yet implemented
- 2. Not set in stone: if you have an opinion, send a PR and we can make a better language together.
- 3. Potentially out of date...

## Language Structure
- Statements end with semicolons (`;`)
    - Automatic Semicolon Insertion: semicolons areoptional, but can help to add meaning
- Not whitespace dependent

## Comments
These get ignored (maybe switch to `#` ?)
- `//` line comments
- `/* ... */` multi-line comments

## Variables/References

### Builtin Global Variables
These are likely to change. All of these values are reassignable and can be referenced and called within other scopes. 
- `i`: command line arguments (Note: only at global scope, see macros section)
- `o`: leave current scope with return value provided as argument (this is known as the return operator in most other langauges)
- `print`: write values to terminal (ie: `print("Hello, world!")`)
- `input`: read values from terminal as a string (ie: `age = Num(input())`)
- `if`: performs functionality of ternary and ifstatements
- `Str`: converts given value to a string representation
- `Num`: Parses a number (output is either Int or Float)
- `vars`: debugging tool
- `async`: run closure in async context (see section)
- `import`: Load a native function or module
- `size`: Gives size of given value, equivalent to `len` in Python
- `copy`: Deep-copies given value


### Declaration
Variables are declared and defined with similar syntax to JavaScript and references behave in a similar way to Java. Not a huge fan of either of these langauges but decided to make something that was predictable and reasonably performant unlike what I made for YodaScript.
```
let name = "John Smith";
let age = 30, vehicle;
vehicle = "Hot rod"; 
```

### WiP: metaprogramming
You can define macros that expand to larger expressions 

## Values
Supports any valid JSON data. Note there are a number of functions 
|Ready|Type|Literal| Use
|---|---|---|---|
|<ul><li>[x] </li></ul>|`Str`|`"Hello, world!"`| Holds character sequences|
|<ul><li>[x] </li></ul>|`Int`|`10`| Holds whole numbers (64 bits) |
|<ul><li>[x] </li></ul>|`Float`|`1.2`| 64bit floating point numbers |
|<ul><li>[x] </li></ul>|`a -> b`|`(: i + 2 )` | first-class functions, alternatives to blocks|
|<ul><li>[x] </li></ul>|`List`|`[1, 2.5, 'cat']`| Hold series of values |
|<ul><li>[x] </li></ul>|`Obj`|`{ temp: 98.6 }`| Similar javascript objects |

## Closures
Closures are first class functions and additionally serve the same functions as code blocks in other languages.

### Defining a Closure
- Closure literals are enclosed in `(:` `)`
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

### Everything is a Closure
Because of the increased tools for control flow, in this langauge, everything is a closure. Where other langauges use operators like `if`, `return`, `await`, etc, this language can just use closures (often even with user-level implementations). Further, this langauge doesn't have blocks (usually in curly braces), because closures can serve the same purpose as them.
### What does this mean?
To emphasize this point further, it can be thought that program files are wrapped in `(:` `)` by the compiler. Command line arguments are passed as the closure's input (`i`) and it's output (`o`) is eqivalent to `sys.exit`. So a simple echo program can be written as such
```
// Echo command line arguments
print(i)
// Exit success
o(0)
```
This concept also applies to modules, exporting a value is as simple as
```
o(3.14159286)

```

## Control Flow
These are currently defined as builtins/standard library functions, but in the future they might be converted to operators.

### Conditionals
For now `if` is just a function.
- comma separated arguments implicitly converted to list
- 
```
let gpa = Num(input()); // 3.86

if (gpa > 4 || gpa < 0, (:
    print("seems rigged");
), gpa >= 2, (:
    print("PASS");
), (:
    print("FAIL");
));
// PASS
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
You can implement `while` on your own like shown below. This will be required while I add conditional jumps to the VM bytecode.
```
let while = (:
    let args = i, break = o
    if (args[0](break), (:
        args[1](break)
        while(args)
    ))
)
```
- Calling `i()` from body or condition will break out of the loop
- Calling `o()` from the body will skip to next cycle
- Calling `i(true)` will make `while(...)` return `true` when you break out

#### Range Based For
The following definitions (among others) will eventually be included in the standard library. In the near future I'll add `range` with similar functionality to python's version but without iterators. 
```
let foreach = (:
	let list = i[0], action = i[1]
	let index = 0, end = size(list)
	while((: index < end), (:
		action(list[index], index, i)
		index = index + 1	
	))
)
let map = (:
	let list = i[0], fn = i[1]
	list = copy(i[0])
	let ret = foreach(i[0], (: list[i[1]] = fn(i) ))
	if(ret == empty, list, ret)
)
```

## More coming soon
Most of these features are at least working, but some may be half baked. There are some things that are implemented haven't made their way into this guide and even more that I haven't implemented but have planned. If there's anything you want to see added, lmk
