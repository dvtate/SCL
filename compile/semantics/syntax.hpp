//
// Created by tate on 02-05-20.
//

#ifndef DLANG_SYNTAX_HPP
#define DLANG_SYNTAX_HPP

#include "process_tree.hpp"


void sem_convert_syntax(AST& t, const std::string& f, std::vector<SemanticError>& errs);


#endif //DLANG_SYNTAX_HPP
