cmake_minimum_required(VERSION 3.17)
project(dlang)

set(CMAKE_CXX_STANDARD 17)

add_executable(dlang main.cpp parse/lex.cpp parse/parse.cpp vm/value.cpp vm/value.hpp vm/gc.cpp vm/gc.hpp compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp compile/semantics/process_tree.hpp compile/semantics/syntax.hpp compile/semantics/syntax.cpp compile/semantics/process_tree.cpp util.cpp util.hpp vm/frame.cpp vm/frame.hpp vm/closure.cpp vm/closure.hpp vm/handle.cpp vm/handle.hpp vm/vm.cpp vm/vm.hpp vm/literal.cpp vm/literal.hpp)