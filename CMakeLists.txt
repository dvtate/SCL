cmake_minimum_required(VERSION 3.17)
project(dlang)

set(CMAKE_CXX_STANDARD 17)

add_executable(dlang main.cpp parse/lex.cpp parse/parse.cpp vm/operator.cpp vm/operator.hpp vm/value.cpp vm/value.hpp vm/rt_macro.cpp vm/rt_macro.hpp vm/gc.cpp vm/gc.hpp vm/reference.cpp vm/reference.hpp compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp compile/semantics/process_tree.hpp compile/semantics/syntax.hpp compile/semantics/syntax.cpp compile/semantics/process_tree.cpp util.cpp util.hpp vm/frame.cpp vm/frame.hpp vm/operators.cpp vm/operators.hpp)