//
// Created by tate on 27-04-20.
//

#ifndef DLANG_PARSE_HPP
#define DLANG_PARSE_HPP

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <variant>

#include "lex.hpp"

class AST {
public:
	enum NodeType {
		STATEMENTS, // semicolon delineated

		// operand types
		OPERATION, // parsed operator args put in this->members
//		EXPRESSION,
		NUM_LITERAL,
		STR_LITERAL,
		IDENTIFIER,
		MACRO_INVOKE,    // fn(arg)
		MEMBER_REQUEST, // bracket operators
		MACRO,	// macro literal
		OBJECT,	// { ... }
		LIST, // [ ... ]

		// containers
		MACRO_OPEN,
		LIST_OPEN,
		PAREN_OPEN,
		CONT_CLOSE,

		// separators
		KV_PAIR,
		COMMA_SERIES,

		BRK_EXPR, // bracket operator
		PAREN_EXPR, // temporary, used to add clarity to macro calls
		OPERATOR,  // un-parsed operator

		DECLARATION,

		INVALID,
	} type;

	Token token;

	std::vector<AST> members;

	// used for reducing constexprs
	// 1 : known volatile
	// 0 : unknown volatility
	// -1 : known const/constexpr
	signed char volatility : 4;

	AST():
		type(INVALID), token(Token()), members(), volatility(0) {}
	AST(const NodeType type, const Token token):
			type(type), token(token), members(), volatility(0) {}
	AST(const NodeType type, const Token token, std::vector<AST> children):
			type(type), token(token), members(std::move(children)), volatility(0) {}


	// std::variant<std::string, int64_t, double> val;

	[[nodiscard]] const std::string type_name() const noexcept {
		switch (this->type) {
			case NodeType::STATEMENTS:		return "Statement sequence";
			case NodeType::OPERATION:		return "Operator Expression";
			case NodeType::NUM_LITERAL:		return "Number Literal";
			case NodeType::STR_LITERAL:		return "String Literal";
			case NodeType::IDENTIFIER:		return "Identifier";
			case NodeType::MACRO_INVOKE:	return "Macro Call";
			case NodeType::MACRO:			return "Macro Literal";
			case NodeType::OBJECT:			return "Object Literal";
			case NodeType::LIST:			return "List";
			case NodeType::MACRO_OPEN:		return "Macro opening";
			case NodeType::LIST_OPEN:		return "List opening";
			case NodeType::PAREN_OPEN:		return "Opening Parenthesis";
			case NodeType::CONT_CLOSE:		return "Container closing";
			case NodeType::KV_PAIR:			return "Key-Value pair";
			case NodeType::COMMA_SERIES:	return "Comma Series";
			case NodeType::BRK_EXPR:		return "Bracketed Expression";
			case NodeType::DECLARATION:		return "Declaration";
			case NodeType::INVALID:			return "Invalid item";
			default:						return "unknown";
		}
	}

};


class SyntaxError {
public:
	Token token;
	std::string msg;

	SyntaxError(Token _token, std::string _message):
			token(std::move(_token)), msg(std::move(_message)) { }
};


AST parse(const std::vector<Token>& tokens);

// Lisp representation
std::string debug_AST(const AST& tree);

#endif //DLANG_PARSE_HPP
