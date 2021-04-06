//
// Created by tate on 01-05-20.
//

#ifndef SCL_BYTECODE_HPP
#define SCL_BYTECODE_HPP

#include <string>
#include <vector>

#include "command.hpp"

// convert to text representation
std::string compile_text(std::vector<Command>);


// convert to compressed binary format
std::vector<char> compile_bin(std::vector<Command>);


#endif //SCL_BYTECODE_HPP
