//
// Created by tate on 01-05-20.
//

#ifndef DLANG_BYTECODE_HPP
#define DLANG_BYTECODE_HPP

#include <string>
#include <vector>

#include "command.hpp"

// convert to text representation
std::string compile_text(std::vector<Command>);


// convert to compressed binary format
// return number of bytes stored in ret
std::size_t compile_bin(std::vector<Command>, char*& ret);


#endif //DLANG_BYTECODE_HPP
