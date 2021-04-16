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
// NOTE: LTR is default
/*
enum class Associativity : signed char { LTR = 0, RTL, NONE };
static std::unordered_map<std::string, Associativity> op_assoc {
		{ ",", Associativity::NONE },
		{ "@", Associativity::NONE },
		{ "=", Associativity::RTL },
		{ "**", Associativity::RTL },
};
*/

static inline AST convert_leaf(AST& t, SemanticContext& ctx)
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


static inline AST convert_branch(AST& t)
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

	// Handle import function
	if (t.type == AST::NodeType::INVOKE) {
		const auto& fxn = t.members[0];
		if (fxn.type == AST::NodeType::IDENTIFIER && fxn.token.token == "import") {
			const auto& file = t.members[1];
			if (file.type != AST::NodeType::STR_LITERAL) {
				errs.emplace_back(SemanticError("import() expected a string literal", file.token.pos, f))
			} else {
				// replace this token with
				// import("./test.scl")
				// becomes
				// (: ... contents of ./test.scl )()
				// TODO ! after dinenner
			}
		}
	}

	// Convert comma operator to COMMA_SERIES
	if (t.type == AST::NodeType::OPERATION && t.token.token == ",")
		t.type = AST::NodeType::COMMA_SERIES;

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
			// non-associative
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