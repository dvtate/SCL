cmake_minimum_required(VERSION 3.17)

project(scl
        LANGUAGES C CXX)
enable_language(CXX)
enable_language(C)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)
link_libraries(dl)

# Debug
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -Wall -Wextra")
# Production
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")

set(SOURCE_FILES
        util.cpp util.hpp debug.hpp

        lib/tsl/ordered_hash.h lib/tsl/ordered_map.hpp
        lib/tsl/ordered_set.hpp

        parse/lex.cpp parse/parse.cpp

        compile/bytecode.hpp compile/bytecode.cpp compile/compile.cpp compile/compile.hpp
        compile/semantics.hpp compile/semantics.cpp compile/tree_to_source.cpp compile/tree_to_source.hpp
        compile/command.hpp compile/command.cpp

        vm/value.cpp vm/value.hpp vm/closure.cpp vm/closure.hpp vm/vm.cpp vm/vm.hpp vm/literal.cpp vm/literal.hpp
        vm/lambda_return.hpp vm/global_ids.cpp vm/global_ids.hpp vm/error.hpp vm/error.cpp vm/async.hpp vm/value_types.hpp
        vm/primitive_methods.cpp vm/primitive_methods.hpp

        vm/bc/read_bc.cpp vm/bc/read_bc.hpp vm/bc/exec_bc_instr.cpp vm/bc/exec_bc_instr.hpp vm/bc/bc.cpp vm/bc/bc.hpp
        vm/bc/fault_table.cpp vm/bc/fault_table.hpp

        vm/operators/operators.cpp vm/operators/operators.hpp vm/operators/math.cpp vm/operators/math.hpp
        vm/operators/internal_tools.cpp vm/operators/internal_tools.hpp vm/operators/cmp.cpp vm/operators/cmp.hpp
        vm/operators/logic.cpp vm/operators/logic.hpp

        vm/gc/gc.cpp vm/gc/gc.hpp

        vm/gc/sync_for_gc.cpp vm/gc/sync_for_gc.hpp)

# VM linking library
add_library(scl_vm SHARED
        ${SOURCE_FILES}
)

# Main binary
add_executable(scl
        main.cpp # ${SOURCE_FILES}
)
target_compile_features(scl PUBLIC cxx_std_20)
target_link_libraries(scl PRIVATE Threads::Threads)
target_link_libraries(scl PUBLIC scl_vm)

######
# Builtin, native libraries
######

add_library(json SHARED
        builtin_libs/json/main.cpp builtin_libs/json/parse.hpp builtin_libs/json/stringify.hpp builtin_libs/json/pretty.hpp builtin_libs/json/cyclic_refs_exception.hpp)
target_link_libraries(json PUBLIC scl_vm)


add_library(time SHARED
        builtin_libs/time/main.cpp builtin_libs/time/delay.hpp builtin_libs/time/timepoint.hpp)
target_link_libraries(json PUBLIC scl_vm)

