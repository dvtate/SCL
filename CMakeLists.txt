cmake_minimum_required(VERSION 3.17)
project(dlang)

set(CMAKE_CXX_STANDARD 17)

add_executable(dlang main.cpp parse/lex.cpp parse/parse.cpp)