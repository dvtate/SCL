//
// Created by tate on 02-05-20.
//

#ifndef DLANG_PROCESS_TREE_HPP
#define DLANG_PROCESS_TREE_HPP


#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>


#include "../../parse/parse.hpp"

/* Semantic Analysis
 *
 * Reduce Syntax:
 * - Convert Builtin-identifiers/constants (ie - print)
 * - Handle operator associativity
 * - Convert complex let expressions
 * 		+ let a = 2, b = 4;
 * 		--->
 * 		+ let a; a = 2; let b; b = 4;
 * - remove PAREN_EXPRs
 * - Convert function calls on CSV's to list
 *
 * Check Safety:
 * - basic typechecking and type deduction
 * - use before declaration (maybe remove in future)
 * -
 *
 * Optimize performance:
 * - Simplify Const-exprs
 * - macro inlining
 * - no construction-call-destruction for arguments
 * - replace object member requests with index operation (compile-time IC)
 */


class SemanticError {
public:
	std::string msg;
	unsigned long long int pos;
	std::string file;
	bool is_warn;
	SemanticError(const std::string& message, const unsigned long long position, const std::string& fname, bool is_warning = false):
			msg(message), pos(position), file(fname), is_warn(is_warning) {}
};


std::vector<SemanticError> process_tree(AST&, std::string);

#endif //DLANG_PROCESS_TREE_HPP
