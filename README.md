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
This will give you a binary which you can do `./scl help` for documentation on how to use. The commands are as follows:
- `build`: generate a bytecode file for given source code
- `exec`: run a bytecode file
- `eval`: compile and run given source code
- `minify`: Minify input source code
- `debug`: generate bytecode text for given source code
Notice that all of these commands take one argument as their input file.

## Language Structure
- Statements end with semicolons (`;`)
    - Automatic Semicolon Insertion: semicolons are optional, but can help to add meaning
- Not whitespace dependent
- Comments: `//` and `/* ... */`

## Variables/References

<details>
  <summary>Builtin Global Variables</summary>
	
All of these values are reassignable and can be referenced and called within other scopes. 
- `i`: command line arguments (Note: only at global scope, see closures section)
- `o`: leave current scope with return value provided as argument (this is known as the return operator in most other languages)
- `print`: write values to terminal (ie: `print("Hello, world!")`)
- `input`: read values from terminal as a string (ie: `age = Num(input())`)
- `if`: performs functionality of ternary and branching
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
|Type|Literal| Use
|---|---|---|
|`Str`|`"Hello, world!"`| Holds character sequences|
|`Int`|`10`| Holds whole numbers (64 bits) |
|`Float`|`1.2`| 64bit floating point numbers |
|`a -> b`|`(: i + 2 )` | first-class functions, alternatives to blocks|
|`List`|`[1, 2.5, 'cat']`| Hold series of values |
|`Obj`|`{ temp: 98.6 }`| Dictionary with strings as keys |

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
This was the main reason I made this language. JavaScript, Python, C· and most other languages featuring the most popular async/await syntax require you to change your program depending on sync/async context and in doing so adds excess and/or confusing features. In this language I only needed to add a single built-in global `async` in order to provide equivalent functionality.

### Running code in a new thread
Lets walk through an exmaple that gets main points across. Imagine we have a function `request` that takes a url and fetches it's content over the internet. 
```
let request = import('request.so')
```
We can call request like any normal function and as we're awaiting the results, the VM can work on other tasks
```
let text = request('http://x.com')
print(text); // x
```
However we can also perform the function call in a separate thread! We first make an `async` wrapper for the `request` function and then call it, receiving an eventual. Which we can call later to get the results.
```
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

## Core values
- Unclear operators should be avoided, use functions instead
	- ie - `return`, `break`, `continue`, `export`, etc. don't exist here
- No redundant language features
- Avoid strong opinions

## More coming soon
Most of these features are at least working. There are some things that are implemented haven't made their way into this guide and even more that I haven't implemented but [have planned](https://docs.google.com/spreadsheets/d/1HZEsRAPhoAOnP-zT70_9bgLTrJVC9NmNyxKYWxsTT8Q/edit?usp=sharing). If there's anything you want to see added, lmk.
