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
std::vector<char> compile_bin(std::vector<Command>);


#endif //DLANG_BYTECODE_HPP
