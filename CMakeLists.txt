cmake_minimum_required(VERSION 3.17)
project(dlang)

set(CMAKE_CXX_STANDARD 20)

add_executable(dlang main.cpp parse/lex.cpp parse/parse.cpp vm/value.cpp vm/value.hpp vm/gc.cpp vm/gc.hpp compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp compile/semantics/process_tree.hpp compile/semantics/syntax.hpp compile/semantics/syntax.cpp compile/semantics/process_tree.cpp util.cpp util.hpp vm/closure.cpp vm/closure.hpp vm/handle.cpp vm/handle.hpp vm/vm.cpp vm/vm.hpp vm/literal.cpp vm/literal.hpp vm/read_bc.cpp vm/read_bc.hpp vm/exec_bc_instr.cpp vm/exec_bc_instr.hpp vm/operators.cpp vm/operators.hpp vm/builtin_operators/math.cpp vm/builtin_operators/math.hpp vm/lambda_return.cpp vm/lambda_return.hpp vm/global_ids.cpp vm/global_ids.hpp)