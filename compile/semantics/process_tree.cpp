//
// Created by tate on 02-05-20.
//

#include "process_tree.hpp"

#include "syntax.hpp"



std::vector<SemanticError> process_tree(AST& t, const std::string f) {

	std::vector<SemanticError> ret;
	ret = sem_convert_syntax(t, f);
}