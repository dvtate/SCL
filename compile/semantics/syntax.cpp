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






// OPTIMIZE: this can be replaced with Preprocessor
enum class Associativity : signed char { LTR, RTL, NONE };
std::unordered_map<std::string, Associativity> op_assoc{
		{ ",", Associativity::NONE },
		{ "@", Associativity::NONE },
		{ "=", Associativity::RTL },
		{ "**", Associativity::RTL },
		{ "+", Associativity::LTR },
		{ "-", Associativity::LTR },
		{ "*", Associativity::LTR },
		{ "/", Associativity::LTR },
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


static inline AST convert_branch(AST& t, std::string f, std::vector<SemanticError>& errs)
{
	if (t.type != AST::NodeType::PAREN_EXPR && t.members.empty())
		return convert_leaf(t, f, errs);

	// convert paren exprs
	if (t.type == AST::NodeType::PAREN_EXPR) {
		if (t.members.size() > 1) {
			errs.emplace_back(SemanticError("Invalid parenthesized expression", t.token.pos, f));
			return t;
		}
		return t.members[0];
	}

	// Convert CSV's
	// if (t.type == AST::NodeType::OPERATOR && t.token.token == ",") { }

		return t;

	}
}


void sem_convert_syntax(AST& t, std::string f, std::vector<SemanticError>& errs)
{

	t = convert_branch(t, f, errs);

}