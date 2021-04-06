//
// Created by tate on 16-04-20.
//

#include "parse.hpp"

/**
 * This file is a fucking spaghetti mess and should be rewritten
 * should also probably rewrite:
 *  - lexer: we have to finish it's job in next_node here
 *  - compile/semantics/syntax:
 *
 * The basic idea is a shift reduce parser with a lookahead token and infinite lookback
 *
 * When I bootstrap compiler, will likely use recursive descent parser
 *
 */



// convert AST to lisp
/// TODO: indentation + depth tracking
std::string debug_AST(const AST& tree) {
	std::string ret = "(";
	ret += tree.token.token;
	ret += "<";
	ret += tree.short_type_name();
	ret += ">";
	for (const AST& m : tree.members)
		ret += " " + debug_AST(m);
	ret += ')';
	return ret;
}

// using a shift-reduce parser
std::vector<std::string> keywords;


// Operator Precedence
std::unordered_map<std::string, signed char> op_prec = {
		{ "(:", 22 },
		{ "{", 22  },
		{ "(",  21 },
		{ "[",  20 },
		{ ".",	21 }, // TODO switch this back to 20 and replace reduce_invoke with operator insertion
		{ "@inv", 20 }, // invoke
		{ "@idx", 20 }, // index
		{ "ref", 18 },
		{ "!", 17 },
		{ "neg", 17 },
		{ "**", 16 },
		{ "*", 	15 },
		{ "/", 	15 },
		{ "%", 	15 },
		{ "+", 	14 },
		{ "-", 	14 },
//		{ "<<", 13 },
//		{ ">>", 13 },
//		{ ">>>", 13 },
		{ "<", 	12 },
		{ ">", 	12 },
		{ "<=", 12 },
		{ ">=", 12 },
		{ "==", 11 },
		{ "===", 11 },
		{ "!=", 11 },
		{ "&", 	10 },
		{ "^", 	9 },
		{ "|", 	8 },
		{ "&&", 6 },
		{ "||", 5 },
//		{ "?", 	4 }, // tern... if ever added..
		{ "=", 	3 }, // assignment
		{ ":", 	2 }, // key value pair
		{ ",", 	1 }, // comma seq
		{ "let", 0 }, // declarations
		{ "using", 0 }, // template engine
		{ "]",  0 },
		{ "}",  0 },
		{ ";",  0 }, // statement separator
		{ ")",  0 },
		{ "eof", 0 }
};

// Is given node a valid expression with a resulting type
//  enum entries ordered specifically to enable this
static inline bool isOperand(const AST& n) {
	return n.type >= AST::NodeType::OPERATION && n.type <= AST::NodeType::LIST;
}

// Get the next AST node from the list of tokens
//   Also finish lexers job as some tokens combine into single nodes
static inline AST next_node(const std::vector<Token>& tokens, size_t& i, std::vector<AST> stack) {
	// Mutable copy of token
	Token t = tokens[i];

	// Lexer error, (ie: unterminated comment/string/etc.)
	if (t.type == Token::t::ERROR) {
		throw std::vector<SyntaxError>{SyntaxError(t, t.token)};
	}

	// recombine multi-char operators
	if (t.type == Token::t::OPERATOR && tokens.size() - i > 1 && tokens[i + 1].type == Token::t::OPERATOR) {
		const std::string combined = tokens[i].token + tokens[i + 1].token;
		static const std::vector<std::string> mc_ops = { "&&", "||", "<=", ">=", "==", "!=", "**" };

		if (combined == "(:") {
			// special case for macro-open (different NodeType)
			t.token = combined;
			const AST ret = AST(AST::NodeType::MACRO_OPEN, t);
			i += 2;
			return ret;

		} else if (std::find(mc_ops.begin(), mc_ops.end(), combined) != mc_ops.end()) {
			// Other multicharacter operator
			t.token = combined;
			i += 2;
			return AST(AST::NodeType::OPERATOR, t );
		}
	}

	// everything past here is only single token
	i++;

	// Literals
	if (t.type == Token::t::NUMBER)
		return AST(AST::NodeType::NUM_LITERAL, t);
	if (t.type == Token::t::STRING)
		return AST(AST::NodeType::STR_LITERAL, t);

	// Handle Identifiers
	if (t.type == Token::t::IDENTIFIER) {
		// These are technically operators
		if (t.token == "let" || t.token == "using") {
			t.type = Token::t::OPERATOR; // prob not needed anymore..
			return AST(AST::NodeType::OPERATOR, t);
		}
		return AST(AST::NodeType::IDENTIFIER, t);
	}

	// TODO Lexer could probably handle some of this logic better
	if (t.type == Token::t::OPERATOR) {
		// Containers
		if (t.token[0] == '(')
			return AST(AST::NodeType::PAREN_OPEN, t);
		if (t.token[0] == '[')
			return AST(AST::NodeType::LIST_OPEN, t);
		if (t.token[0] == '{')
			return AST(AST::NodeType::OBJ_OPEN, t);
		if (t.token[0] == ')' || t.token[0] == ']' || t.token[0] == '}')
			return AST(AST::NodeType::CONT_CLOSE, t);

		// unary minus (negation)
		if (t.token == "-" && !isOperand(stack.back())) {
			t.token = "neg"; // Distinct name for precedence and compilation
			return AST(AST::NodeType::OPERATOR, t);
		}

		// eof only purpose is to force expression reducing
		if (t.token == "eof")
			return AST(AST::NodeType::INVALID, t);

		// Otherwise it's just a basic operator like +,-,*,/ etc.
		return AST(AST::NodeType::OPERATOR, t);
	}

	throw std::vector<SyntaxError>{SyntaxError(t, "Unknown token")};
}

/** Find index of corresponding container open node on stack
 * @param stack - parse stack
 * @param nn - container closing
 * @returns -1 if passed invalid close character
 * @throws {SyntaxError} if no matching container start
 */
static inline int prev_matching_container_index(std::vector<AST>& stack, const AST& nn) {
	if (nn.type != AST::NodeType::CONT_CLOSE)
		return -1;
	std::size_t ret = stack.size();

	// Find macro/paren_expr open
	if (nn.token.token[0] == ')')
		while (--ret >= 0)
			if (stack[ret].type == AST::NodeType::MACRO_OPEN ||
				stack[ret].type == AST::NodeType::PAREN_OPEN)
				return ret;

	// Else Find list open
	if (nn.token.token[0] == ']')
		while (--ret >= 0)
			if (stack[ret].type == AST::NodeType::LIST_OPEN)
				return ret;

	// Else find object open
	if (nn.token.token[0] == '}')
		while (--ret >= 0)
			if (stack[ret].type == AST::NodeType::OBJ_OPEN)
				return ret;

	// closing without opener
	throw std::vector<SyntaxError>{SyntaxError(nn.token,
			"unexpected " + nn.type_name() + ": " + nn.token.token)};
}

/**
 * Get index of last operator on stack
 * @param stack - Parse stack
 * @param n - lookahead node
 * @return - index of operator or -1 if none found
 */
static inline int prev_operator(std::vector<AST>& stack, const AST& lookahead) {
	int min_i = 0;
	// only reduce operators within current container scope (ie - parens)
	if (lookahead.type == AST::NodeType::CONT_CLOSE) {
		min_i = prev_matching_container_index(stack, lookahead);
	}

	// Find next operator
	ssize_t ret = stack.size();
	while (--ret >= min_i)
		if (stack[ret].type == AST::NodeType::OPERATOR)
			return ret;

	return -1;
}

/**
 * Attempt to link operator at index i on the stack with it's operands
 *  there are many problems with this function but it works for the most part
 *
 * @param stack - parse stack
 * @param i - index
 * @return nullptr on success and error message on failure
 */
static inline const char* reduce_operator(std::vector<AST>& stack, const size_t i) {
	// all operators are binary except: "neg", "let", and !
	// Lexical operators: , ; :
	// we will only ever reduce the last operator on the stack
	// 	otherwise we wait for something with lower precedence to get placed on
	//  Might not be a good idea to rely on this but idk

	AST n = stack[i];
	const std::string op = n.token.token;

	// return kv pair
	if (op == ":") {
		if (stack.size() - i != 2 || stack.size() < 3)
			return "invalid colon";

		const AST r = stack.back();
		stack.pop_back();
		stack.pop_back();
		const AST l = stack.back();
		n.type = AST::NodeType::KV_PAIR;
		n.members.emplace_back(l);
		n.members.emplace_back(r);
		stack.back() = n;
		return nullptr;
	}

	// unary prefix
	if (op == "neg" || op == "!") {
		if (stack.size() - i != 2)
			return "invalid unary operation";
		n.type = AST::OPERATION;
		n.members.emplace_back(stack.back());
		stack.pop_back();
		stack.back() = n;
		return nullptr;
	}

	// unary prefix declaration
	if (op == "let") {
		if (stack.size() - i != 2)
			return "invalid declaration";
		n.type = AST::NodeType::DECLARATION;
		n.members.emplace_back(stack.back());
		stack.pop_back();
		stack.back() = n;
		return nullptr;
	}

	// postfix semicolon
	if (op == ";") {

		// drop semi
		stack.pop_back();

		// allow empty-statements ;;;;;;
		if (stack.empty())
			return nullptr;
		if (stack.back().type == AST::NodeType::STATEMENTS)
			return nullptr;

		// Make a new statements node
		AST expr = stack.back();
		stack.pop_back();
		if (stack.empty() || stack.back().type != AST::NodeType::STATEMENTS) {
			stack.emplace_back(AST(
					AST::NodeType::STATEMENTS,
					Token(Token::t::OPERATOR, ";", expr.token.pos),
					{ expr }));
			return nullptr;
		}
		// Combine with previous statements
		if (stack.back().type == AST::NodeType::STATEMENTS)
			stack.back().members.emplace_back(expr);
		return nullptr;

	}

	// generic infix operation
	if (stack.size() - i != 2 || stack.size() < 3) {
		//throw std::vector<SyntaxError>{SyntaxError(n.token, "Invalid infix operator...")};
		return "invalid infix operator";
	}

	// convert paren exprs
	if (stack.back().type == AST::NodeType::PAREN_EXPR) {
		auto m = stack.back().members[0];
		if (isOperand(m))
			stack.back() = m;
	}

	// invalid operand
	if (!isOperand(stack.back())) {
#ifdef SCL_DEBUG
		std::cout <<"\nnot an operand: `" <<stack.back().token.token <<"::" <<stack.back().short_type_name()
		<<"` n=`" <<n.token.token <<"::" <<n.short_type_name() <<"`\n";
#endif
		return "invalid operand";
	}

	// get two operands (left and right)
	const AST r = stack.back();
	stack.pop_back(); // pop r
	stack.pop_back(); // pop n
	const AST l = stack.back();

	// OPERATION is valid operand, OPERATOR is not
	n.type = AST::NodeType::OPERATION;

	// if multiple members of same type, combine into bulk operation
	//    we can deal with associativity later
	// 1+2+3+4-5-6  =>  (+ 1 2 3 (- 4 5 6))
	if (l.type == AST::NodeType::OPERATION && l.token.token == n.token.token)
		n.members = l.members;
	else
		n.members.emplace_back(l);

	if (r.type == AST::NodeType::OPERATION && r.token.token == n.token.token)
		n.members.insert(n.members.end(), r.members.begin(), r.members.end());
	else
		n.members.emplace_back(r);

	// replace l with our new operation
	stack.back() = n;
	return nullptr;
}


/**
 * Try to reduce expressions involving operators ((1)(+)(2) => (+ 1 2)
 * @param stack - parse stack
 * @param n - lookahead
 * @return - true/false depending on if a reduction was made
 */
static inline bool reduce_operators(std::vector<struct AST>& stack, const AST& n) {
	// dont reduce if given neg
	if (n.type == AST::NodeType::OPERATOR && n.token.token == "neg")
		return false;

	// lookahead is an operator
	if (!isOperand(n)) {
		// find last operator
		const int i = prev_operator(stack, n);
		if (i < 0)
			return false;

		// Get precedences
		const auto prec_p = op_prec.at(stack[i].token.token);
		const auto prec_n = op_prec.at(n.token.token);

		// reduce if lookahead is lower precedence than prev operator
		if (prec_n <= prec_p || stack[i].token.token == ";") {
			// NOTE reduction can fail in normal course of parse ://///
			const auto ret = reduce_operator(stack, i);
			SCL_DEBUG_MSG("Parser reduce operator(" << stack[i].token.token << "): " << (ret ? ret : "null") << std::endl);

			return !ret;
		} else {
			// lookahead is higher precidence, need to reduce it before can reduce prev
			return false;
		}
	}

	// double operands
	// assumed/given end of expr, reduce it before adding more
	// TODO this is also the condition for ASI
	if ((n.type != AST::NodeType::LIST_OPEN && n.type != AST::PAREN_OPEN && isOperand(stack.back()))
		|| (stack.back().token.token == ";" && stack.back().type != AST::NodeType::STATEMENTS)) {
		// find previous operator (if any)
		auto p_op_i = prev_operator(stack, n);
		if (p_op_i < 0)
			return false;

		// try to reduce it
		const auto ret = reduce_operator(stack, p_op_i);
		SCL_DEBUG_MSG("Parser ASI reduce operator(" << stack[p_op_i].token.token << "): " << (ret ? ret : "null") << std::endl);
		return !ret;
	}

	// No reduce
	return false;
}
/**
 * Reduce containers replacing (open) (body) (close) with (container (body))
 * @param stack - parse stack
 * @return - true/false depending on if a reduction was made
 */
static inline bool reduce_containers(std::vector<AST>& stack) {
	// Need to see a container close to be able to reduce
	if (!stack.empty() && stack.back().type == AST::NodeType::CONT_CLOSE) {
		// find nearest opener
		int i = (int) stack.size();
		while (--i >= 0)
			if (stack[i].type == AST::NodeType::LIST_OPEN ||
				stack[i].type == AST::NodeType::MACRO_OPEN ||
				stack[i].type == AST::NodeType::PAREN_OPEN ||
				stack[i].type == AST::NodeType::OBJ_OPEN)
				break;

		// No matching opener found!
		if (i < 0)
			throw std::vector<SyntaxError>{SyntaxError(stack.back().token,
					"Unexpected " + stack.back().token.token + " missing matching open")};

		// handle lists
		if (stack[i].type == AST::NodeType::LIST_OPEN) {
			// mismatch
			if (stack.back().token.token != "]")
				throw std::vector<SyntaxError>{
					SyntaxError(stack[i].token, "unclosed `[`"),
					SyntaxError(stack.back().token,
					std::string("unexpected `") + stack.back().token.token + '`'),
				};
			// Should only be one node in container
			if (stack.size() - i > 3)
				throw std::vector<SyntaxError>{SyntaxError(stack[i].token, "invalid expression in list")};

			stack.pop_back(); // pop close
			AST n = stack.back();
			stack.pop_back(); // pop inside

			// list.members = comma separated values
			if ((n.type == AST::NodeType::OPERATION && n.token.token == ",") || n.type == AST::NodeType::COMMA_SERIES)
				n.type = AST::NodeType::LIST;
			else
				n = AST(AST::NodeType::LIST, stack.back().token, std::vector<AST>({n}));

			// replace start of container with new container node
			stack.back() = n;
			return true;
		}

		// handle parens
		if (stack[i].type == AST::NodeType::PAREN_OPEN) {
			// handle mismatch
			if (stack.back().token.token != ")")
				throw std::vector<SyntaxError>{
					SyntaxError(stack[i].token, "unclosed `(`"),
					SyntaxError(stack.back().token,
						std::string("unexpected `") + stack.back().token.token + '`'),
				};
			if (stack.size() - i > 3)
				throw std::vector<SyntaxError>{SyntaxError(stack[i].token, "invalid expression in parens")};

			// pop closer
			stack.pop_back();

			if ( (long int) stack.size() - 2 == i) {
				// has expression to capture
				const AST e = stack.back();
				stack.pop_back();
				stack.back().type = AST::NodeType::PAREN_EXPR;
				stack.back().members.emplace_back(e);
			} else {
				// () empty parenexpr
				stack.back().type = AST::NodeType::PAREN_EXPR;
			}


			return true;
		}

		// handle macros
		if (stack[i].type == AST::NodeType::MACRO_OPEN) {
			// handle mismatch
			if (stack.back().token.token != ")")
				throw std::vector<SyntaxError>{
					SyntaxError(stack[i].token, "unclosed `(:`"),
					SyntaxError(stack.back().token,
						std::string("unexpected `") + stack.back().token.token + '`'),
				};
			stack.pop_back();
			for (unsigned short m = i + 1; m < stack.size(); m++)
				stack[i].members.emplace_back(stack[m]);
			while (stack.size() > i + 1U)
				stack.pop_back();
			stack.back().type = AST::NodeType::MACRO;

			return true;
		}


		// Handle Objects
		if (stack[i].type == AST::NodeType::OBJ_OPEN) {
			if (stack.back().token.token != "}")
				throw std::vector<SyntaxError>{
					SyntaxError(stack[i].token, "unclosed `{`"),
					SyntaxError(stack[i].token,
						std::string("unexpected `") + stack.back().token.token + '`')
				};

			if (stack.size() - i > 3)
				throw std::vector<SyntaxError>{SyntaxError(stack[i].token, "invalid expression in parens")};

			// pop closer
			stack.pop_back();
			if ( (long int) stack.size() - 2 == i) {
				// has expression to capture
				const AST e = stack.back();
				stack.pop_back();
				stack.back().type = AST::NodeType::OBJECT;
				stack.back().members.emplace_back(e);
			} else {
				// {} empty obj
				stack.back().type = AST::NodeType::OBJECT;
			}
			return true;
		}
	}

	return false;
}

// macro calls			abc(123)
// bracket operator 	abc[123]
static inline bool reduce_invocations(std::vector<AST>& stack) {
	/* macro calls
	 * takes something that evaluates to a macro
	 * macro literal, paren expr,
	 */

	// try reduce macro calls
	if (stack.back().type == AST::NodeType::PAREN_EXPR) {
		if (stack.size() < 2)
			return false;

		const AST arg = stack.back();
		if (isOperand(stack[stack.size() - 2])) {
			stack.pop_back();

			std::vector<AST> children { stack.back() };
			if (!arg.members.empty()) {
				children.emplace_back(arg.members[0]);
				// if they pass csv args convert them to list
				if (children.back().type == AST::NodeType::OPERATION && children.back().token.token == ",")
					children.back().type =  AST::NodeType::LIST;
			}
			if (arg.members.size() > 1)
				throw std::vector<SyntaxError>{SyntaxError(arg.token, "Invalid parenthesised expression")};

			stack.back() = AST(AST::NodeType::INVOKE, Token(Token::t::OPERATOR, "@"), children);
		}
	}

	// try reduce indexing
	if (stack.back().type == AST::LIST) {
		if (stack.size() < 2)
			return false;
		if (isOperand(stack[stack.size() - 2])) {
			AST ind(AST::NodeType::INDEX, stack.back().token);
			ind.members.emplace_back(stack[stack.size() - 2]);
			ind.members.emplace_back(stack.back().members.back()); // expr inside brackets
			stack.pop_back();
			stack.back() = ind;
		}
	}

	return false;
}

static inline bool reduce_asi(std::vector<AST>& stack, const AST& n) {
	if (n.type != AST::NodeType::INVALID || stack.size() == 1 || n.token.token != "eof")
		return false;

	// Verify that all tokens are fully parsed
	for (auto n : stack)
		if (n.type > AST::NodeType::LIST)
			return false;

	// Put them all in a global statements node
	AST program = n;
	program.type = AST::NodeType::STATEMENTS;
	program.members = stack;
	stack.clear();
	stack.emplace_back(program);
	return true;
}

// Follow rules to form parse tree
static inline bool reduce(const std::vector<Token>& tokens, size_t& i, std::vector<AST>& stack, const AST& n) {
	if (stack.empty())
		return false;

	// TODO ASI if <eof> or <;> try to reduce to single statements token (stack.insert(...))
	return
			reduce_invocations(stack)
			||
			reduce_operators(stack, n)
			||
			reduce_containers(stack)
			||
			reduce_asi(stack, n);
}

// [placeholder] Can we put node onto stack?
static inline bool can_shift(const AST& n) {
	return n.type != AST::NodeType::INVALID;
}


/// Read through tokens shifting and reducing as needed
/// @return parse tree
AST parse(const std::vector<Token>& tokens) {
	std::vector<AST> stack;

	size_t i = 0;

	while (i < tokens.size()) {
		// get lookahead
		AST tok = next_node(tokens, i, stack);
#ifdef SCL_DEBUG
		std::cout <<"Lookahead: " << debug_AST(tok) <<std::endl;
		do {
			std::cout <<"Stack: ";
			for (const AST& n : stack)
				std::cout <<debug_AST(n) << "   ";
			std::cout <<std::endl;
		}
#endif
		// call reduce until it stops making changes
		while (reduce(tokens, i, stack, tok));

		// shift node onto parse stack
		if (can_shift(tok))
			stack.emplace_back(tok);
	}
#ifdef SCL_DEBUG
	std::cout <<"\nStack: ";
	for (const AST& n : stack)
		std::cout <<debug_AST(n) << "   ";
	std::cout <<std::endl;
#endif

	// Parsed multiple items... probably syntax error and/or parse poorly written
	if (stack.size() > 1) {
		// Dump stack
		std::cout <<"\nStack: ";
		for (const AST& n : stack)
			std::cout <<debug_AST(n) << "   ";
		std::cout <<std::endl;

		// Point to everything on stack in their code
		// TODO mark statements groups as safe and/or other regions as bad
		std::vector<SyntaxError> errs;
		for (auto& t : stack)
			errs.emplace_back(SyntaxError(t.token,
					"Unable to parse to single item (maybe bad syntax here?)"));
		throw errs;
	}

	// return top of stack
	return stack.back();
}