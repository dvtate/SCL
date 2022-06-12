//
// Created by tate on 18-05-20.
//

#ifndef SCL_READ_BC_HPP
#define SCL_READ_BC_HPP


#include <istream>
#include "../../debug.hpp"
#include "../literal.hpp"

/// middle section where user code lives
std::vector<Literal> read_lit_header(std::istream& is);

// TODO: metadata section   (top)

#endif //SCL_READ_BC_HPP
