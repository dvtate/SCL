# SCL - Simple Callback Language
This language was my attempt to solve some of the pains I experienced writing internet facing JavaScript and Python software. The acronym and file extension is SCL so the language can also be pronounced like 'Sickle'.

## Build
The only system requirements should be a modern C++ compiler as I've only used things in the standard library. 
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

### Compile
#### Bytecode Text
Useful for debugging compiler. Also prints compile errors/warnings.
<details>
	<summary>See demo</summary>

The following example shows the bytecode text for `print("Hi")`
	
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

</details>

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
    - Automatic Semicolon Insertion: semicolons are optional, but can help to add meaning
- Not whitespace dependent
- Comments: `//` and `/* ... */`

## Variables/References

<details>
  <summary>Builtin Global Variables</summary>
	
All of these values are reassignable and can be referenced and called within other scopes. 
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
	
</details>

### Declaration
Variables are declared with the let operator which has similar syntax to JavaScript. References also behave as you would expect from JavaScript.
```
let name = "John Smith";
let age = 30, vehicle;
vehicle = "Hot rod"; 
```

### WiP: metaprogramming
You can define macros that expand to larger expressions 

## Values
Supports any valid JSON data 
|Ready|Type|Literal| Use
|---|---|---|---|
|<ul><li>[x] </li></ul>|`Str`|`"Hello, world!"`| Holds character sequences|
|<ul><li>[x] </li></ul>|`Int`|`10`| Holds whole numbers (64 bits) |
|<ul><li>[x] </li></ul>|`Float`|`1.2`| 64bit floating point numbers |
|<ul><li>[x] </li></ul>|`a -> b`|`(: i + 2 )` | first-class functions, alternatives to blocks|
|<ul><li>[x] </li></ul>|`List`|`[1, 2.5, 'cat']`| Hold series of values |
|<ul><li>[x] </li></ul>|`Obj`|`{ temp: 98.6 }`| Similar javascript objects |

## Closures
Closures are first class functions but more important here as they're used to replace code blocks.

### Defining a Closure
- Closure literals are enclosed in `(:` `)`
- Variables can reference macros just like any other data, however code cannot modify their internals
```
let say_hello = (:
    print("hello, " + i);
);

print("what's your name?");
let name = input();
say_hello(name); // greets user
```

### Input and Output
Input is accessible via the local variable `i`. Use the local variable `o` to return a value. Although you can use i and o themselves, declaring variables (`let`) or aliases (`using`) for them can improve clarity and is required when they get shadowed by a previous scope.
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
This simplicity also applies to modules. There's no reason to have special operator for exports.
```
o(3.14159286)
```

## Control Flow
These are currently defined as builtins/standard library functions, but in the future they might be converted to operators.

### Conditionals
For now `if` is just a function.
- Note: comma separated arguments implicitly converted to list
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

You could also implement a less useful `if` like so
```
let tern =  (: i[1 + Int(i[0] != 0)] );
let if = (: tern(i)() );
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
You can implement `while` on your own like shown below. This will be required until conditional jumps get added to the VM bytecode.
```
let while = (:
    let args = i, break = o
    if (args[0](break), (:
        args[1](break)
        while(args)
    ))
)
```
- Calling `o()` from the body will skip to next cycle
- Calling `i()` from body or condition will break out of the loop
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

## Async
This for me was the main reason I made this language, javascript, python, C#, and most other languages featuring eventloops require you to change your logic depending on whether the code is running in a sync vs async environment and doing so adds excess language features that ultimately aren't user friendly. Instead I've added async to this language by simply adding a single builtin global `async`. And the user can use other langauge features to get equivalent functionality.

### Running code in a new thread
```
// Imagine request is a function that takes a url and fetches
// it's content from the internet
let request = import('request.so')

// We can call request just like any other function
// The VM will put this thread on hold and do other tasks while wait
let text = request('http://x.com')
print(text); // x

// Alternatively we can make the request in a new thread
let eventual = async(request)('http://x.com')

// So that we can do other things while we wait on the download
print('waiting...')

// And then we can simply invoke the eventual to get the same behavior as before
print(eventual()) // x
```

### Callbacks
See [async demo](https://github.com/dvtate/simple-callback-language/blob/master/examples/async_demo.s) to see how easy it is to convert between functions that return promises and functions with callbacks

### Hanging
By default functions will implicitly return when they reach the end, however this behavior can be overridden by changing the value of `o`.
##### Notes
- This is dangerous because the thread won't return unless you already passed `o` to something that can explicitly call it. 
- Feature may be removed in future implementation and is usually wrong to use

```
// Function that freezes thread for given duration
let delay = import(delay)

// This is equivalent to JavaScript's window.setTimeout
let set_timeout = (:
	let duration = i[0], action = i[1], arg = i[2]
	async((:
		delay(duration)
		action(arg)
	))()
)

// This function does same thing as `delay`
let delay2 = (:
	// After i ms, set_timeout will call o
	set_timeout(i, o)
	
	// Prevent implicit return
	o = 0
)

// Note that in this example, it's reccomended to simply do
let delay3 = (: set_timeout(i, o)() )
```

### Yielding
TODO

### Core values
- Unclear operators should be avoided, use functions instead
	- ie - `return`, `break`, `continue`, `export`, `import`, etc.
- No redundant language features
	- User should be able to create their own tools to improve clarity or use those from a standard library
- Don't force opinions on the user

## Duct Tape and Bubble Gum
Targets for refactoring
### Garbage Collection
The language features a stop the world, tracing garbage collector. Because my goal was to make a cool language and not an impressive GC, the current implementation is a pretty bad rough draft. Eventually I'd like to make it directly manage the heap instead of using `malloc`/`free` and there's some other gross things going on. I think I could easily double the performance of this rough draft but it's not a high priority at the moment.

### Parser
I made a custom shift-reduce parser that works for most programs, however because I didn't write a fully speccd out language grammer or use a compiler-compiler, there are likely some edge cases that I did't consider and performance could be better.

## More coming soon
Most of these features are at least working. There are some things that are implemented haven't made their way into this guide and even more that I haven't implemented but [have planned](https://docs.google.com/spreadsheets/d/1HZEsRAPhoAOnP-zT70_9bgLTrJVC9NmNyxKYWxsTT8Q/edit?usp=sharing). If there's anything you want to see added, lmk.
