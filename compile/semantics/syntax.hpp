//
// Created by tate on 02-05-20.
//

#ifndef SCL_SEMANTICS_SYNTAX_HPP
#define SCL_SEMANTICS_SYNTAX_HPP

#include "process_tree.hpp"

// Basically finish parser's job
void sem_convert_syntax(AST& t, const std::string& f, std::vector<SemanticError>& errs);

#endif //SCL_SEMANTICS_SYNTAX_HPP
