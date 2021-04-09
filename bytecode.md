# Bytecode documentation
This is subject to change and might already be outdated. Eventually will have better documentation


## Instructions

### Literal Header
These instructions define literals
- **TODO** make literal strings and json specify their length. For now they expect to be null-terminated
#### `END_LIT_STRING`

#### `START_LIT_STRING`

#### `START_LIT_MACRO`

#### `START_LIT_JSON`

#### `END_LIT_SECTION`

#### `END_LIT_MACRO`

### Macro Bodies
These are like proper VM instructions for control flow and stuff

#### `I63_LIT` 
push int literal onto stack
- Arg: int64

#### `F64_LIT`
push a float literal onto the stack
- Arg: double

#### `DECL_ID`
declare an identifier
- Arg: int64, idid

#### `SET_ID`
Assign a value to an identifier
- Arg: int64, idid
  
#### `USE_ID`
Get value at identifier
- arg: int64, idid
  
#### `USE_LIT` 
push literal onto stack
- arg: int64, lit id
  
#### `BUILTIN_OP` 
operate on stack
- arg: int16, builtin operator id
  
#### `KW_VAL`
builtin keyword-literal (remove?)
- arg: int16
  
#### `CLEAR_STACK`
semicolon operator
- no arg
  
#### `MK_LIST`
constructs a list from values on stack
- arg: int32, number of items to pull from stack

#### `USE_MEM_L`
gets member from an object (using literal number for mem name)
- arg: lit id int64
- stack args: <obj> USE_MEM_L(lit_num)

#### `SET_MEM_L`
sets object member (using literal number for mem name)
arg: lit id int64
stack args: <obj> <value> SET_MEM_L(lit_num)

#### `MK_OBJ`
construct object from values on stack
- arg: number of items to pull from stack

#### `VAL_EMPTY`
Empty, like undefined in js
#### `VAL_TRUE`
True
#### `VAL_FALSE`
False

#### `INVOKE`
call a function
- stack args: <i> <fn> INVOKE

#### `USE_INDEX`
Get value from array
- stack args: <list> <idx> USE_INDEX

#### `SET_INDEX`
Set value in array
- stack args: <list> <index> <value> SET_INDEX

### Fault Table
This section is generated in case of errors

#### `ID_NAME`
user-defined identifier name
- arg: str
  
#### `ID_ID`
compiled identifier number
- arg: int64
  
#### `FILE_NAME`
file path
- arg: str

#### `DEST_POS`
location in compiled file
- arg: int64

#### `SRC_POS`
original file position
- arg: int64