//
// Created by tate on 18-05-20.
//

#ifndef DLANG_READ_BC_HPP
#define DLANG_READ_BC_HPP


#include <istream>

#include "literal.hpp"


std::vector<Literal> read_lit_header(std::istream& is);

// TODO: read fault table...

#endif //DLANG_READ_BC_HPP
