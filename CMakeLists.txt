cmake_minimum_required(VERSION 3.17)
project(scl
        LANGUAGES C CXX)
enable_language(CXX)
enable_language(C)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
link_libraries(dl)

#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lefence -g -Wall -Wextra")
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

add_executable(scl
        main.cpp util.cpp util.hpp debug.hpp

        lib/tsl/ordered_hash.h lib/tsl/ordered_map.hpp
        lib/tsl/ordered_set.hpp

        parse/lex.cpp parse/parse.cpp

        compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp
        compile/semantics.hpp compile/semantics.cpp compile/tree_to_source.cpp compile/tree_to_source.hpp
        compile/command.hpp compile/command.cpp

        vm/value.cpp vm/value.hpp vm/closure.cpp vm/closure.hpp vm/vm.cpp vm/vm.hpp vm/literal.cpp vm/literal.hpp
        vm/lambda_return.hpp vm/global_ids.cpp vm/global_ids.hpp vm/error.hpp vm/error.cpp vm/async.hpp vm/value_types.hpp

        vm/bc/read_bc.cpp vm/bc/read_bc.hpp vm/bc/exec_bc_instr.cpp vm/bc/exec_bc_instr.hpp vm/bc/bc.cpp vm/bc/bc.hpp
        vm/bc/fault_table.cpp vm/bc/fault_table.hpp

        vm/operators/operators.cpp vm/operators/operators.hpp vm/operators/math.cpp vm/operators/math.hpp
        vm/operators/internal_tools.cpp vm/operators/internal_tools.hpp vm/operators/cmp.cpp vm/operators/cmp.hpp
        vm/operators/logic.cpp vm/operators/logic.hpp

        vm/gc/gc.cpp vm/gc/gc.hpp

        vm/primitive_methods.cpp vm/primitive_methods.hpp)


target_link_libraries(scl PRIVATE Threads::Threads)
