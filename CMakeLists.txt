cmake_minimum_required(VERSION 3.17)
project(dlang)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -march=native")

add_executable(dlang main.cpp parse/lex.cpp parse/parse.cpp vm/value.cpp vm/value.hpp vm/gc/gc.cpp vm/gc/gc.hpp compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp compile/semantics/process_tree.hpp compile/semantics/syntax.hpp compile/semantics/syntax.cpp compile/semantics/process_tree.cpp util.cpp util.hpp vm/closure.cpp vm/closure.hpp vm/gc/handle.cpp vm/gc/handle.hpp vm/vm.cpp vm/vm.hpp vm/literal.cpp vm/literal.hpp vm/bc/read_bc.cpp vm/bc/read_bc.hpp vm/bc/exec_bc_instr.cpp vm/bc/exec_bc_instr.hpp vm/operators/operators.cpp vm/operators/operators.hpp vm/operators/math.cpp vm/operators/math.hpp vm/lambda_return.cpp vm/lambda_return.hpp vm/global_ids.cpp vm/global_ids.hpp vm/bc/bc.cpp vm/bc/bc.hpp vm/operators/refs.cpp vm/operators/refs.hpp debug.hpp vm/operators/internal_tools.cpp vm/operators/internal_tools.hpp vm/operators/cmp.cpp vm/operators/cmp.hpp vm/operators/logic.cpp vm/operators/logic.hpp)

target_link_libraries(dlang PRIVATE Threads::Threads)
