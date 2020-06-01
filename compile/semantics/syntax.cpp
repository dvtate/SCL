//
// Created by tate on 02-05-20.
//

#include "syntax.hpp"

/*
* - Convert Builtin-identifiers/constants (ie - print)
* - Handle operator associativity
* - Convert complex let expressions
* 		+ let a = 2, b = 4;
* 		--->
* 		+ let a; a = 2; let b; b = 4;
* - remove PAREN_EXPRs
* - Convert function calls on CSV's to list
*/

// TODO: implicit returns from functions that don't reference `o`


// OPTIMIZE: this can be replaced with Preprocessor
enum class Associativity : signed char { LTR, RTL, NONE };
std::unordered_map<std::string, Associativity> op_assoc{
		{ ",", Associativity::NONE },
		{ "@", Associativity::NONE },
		{ "=", Associativity::RTL },
		{ "**", Associativity::RTL },
};

static inline AST convert_leaf(AST& t, std::string f, std::vector<SemanticError>& errs)
{
//	if (t.type == AST::NodeType::IDENTIFIER) {
//		if (t.token.token == "print") {
//
//		} else if (t.token.token == "input") {
//
//		}
//	}
	return t;
}



static inline AST convert_branch(AST& t, const std::string& f, std::vector<SemanticError>& errs)
{
	if (t.type != AST::NodeType::PAREN_EXPR && t.members.empty())
		return convert_leaf(t, f, errs);

	// convert paren exprs
	if (t.type == AST::NodeType::PAREN_EXPR) {
		if (t.members.size() > 1) {
			errs.emplace_back(SemanticError("Invalid parenthesized expression", t.token.pos, f));
			return t;
		}
		return convert_branch(t.members[0], f, errs);
	}

	// associativity... ideally would have been handled by parser :(
	if (t.type == AST::NodeType::OPERATION && t.members.size() > 2) {
		const std::string& op_sym = t.token.token;

		if (op_sym == ":=" || op_sym == "=" || op_sym == "**") {
			// right associative operators (a = (b = 5))
			AST mem = t;
			mem.members.clear();
			mem.members.emplace_back(t.members.back());
			t.members.pop_back();
			for (unsigned i = t.members.size(); i > 0; i--) {
				mem.members.emplace_back(t.members[i]);
				AST tmp = (mem);
				mem = t;
				mem.members.clear();
				mem.members.emplace_back(tmp);
			}
			mem.members.emplace_back(t.members[0]);
			t = mem;

		} else if (op_sym == "@" || op_sym == ",") {
			// non-associative operators (1,2,3,4,5)
			// no action
		} else {

			// left-associative ((1+2)+3)
			const auto nn = AST(AST::NodeType::OPERATION, t.token);
			AST ret = t;
			ret.members.clear();
			AST* np = &ret;
			for (auto i = t.members.size() - 1; i > 0; i--) {
				np->members.push_back(nn);
				np->members.push_back(t.members[i]);
				np = &np->members[0];
			}
			*np = t.members[0];
			t = ret;

		}
	}

	if (t.members.empty())
		return convert_leaf(t, f, errs);

	// map members
	AST ret = t;
	for (unsigned int i = 0; i < t.members.size(); i++) {
		ret.members[i] = convert_branch(t.members[i], f, errs);
	}
	return ret;

}


void sem_convert_syntax(AST& t, const std::string& f, std::vector<SemanticError>& errs)
{

	t = convert_branch(t, f, errs);

}