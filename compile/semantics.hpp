//
// Created by tate on 02-05-20.
//

#ifndef SCL_SEMANTICS_HPP
#define SCL_SEMANTICS_HPP

#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>

#include "../parse/parse.hpp"

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

// TODO make this use the token instead
class SemanticError {
public:
	std::string msg;
	unsigned long long int pos;
	std::filesystem::path file;
	bool is_warn;

	// if only I could just do
	// template <typename FILE_PATH_T> ...
	SemanticError(
			const std::string& message,
			const unsigned long long position,
			const char* file_name,
			bool is_warning = false):
			msg(message), pos(position), file(file_name), is_warn(is_warning) {}
	SemanticError(
			const std::string& message,
			const unsigned long long position,
			std::string file_name,
			bool is_warning = false):
			msg(message), pos(position), file(file_name), is_warn(is_warning) {}
	SemanticError(
			const std::string& message,
			const unsigned long long position,
			std::filesystem::path file = std::filesystem::current_path(),
			bool is_warning = false):
			msg(message), pos(position), file(file), is_warn(is_warning) {}
	SemanticError(
			const std::string& message,
			const Token& t,
			bool is_warning = false):
			msg(message), pos(t.pos), file(t.file), is_warn(is_warning) {}
};

std::vector<SemanticError> process_tree(AST& tree);

#endif //SCL_SEMANTICS_HPP
