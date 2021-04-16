//
// Created by tate on 27-04-20.
//

#ifndef SCL_PARSE_HPP
#define SCL_PARSE_HPP

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <variant>

#include "lex.hpp"

class AST {
public:
	enum NodeType {
		// semicolon delineated expressions
		STATEMENTS = 0,

		// result of a let expr
		DECLARATION,

		// operand types (must start with OPERATION and end with LIST)
		OPERATION, // parsed operator args put in this->members
//		EXPRESSION,
		NUM_LITERAL,
		STR_LITERAL,
		IDENTIFIER,
		INVOKE,    // fn(arg)
		INDEX,
		MEMBER_REQUEST, // bracket operators
		MACRO,	// macro literal
		PAREN_EXPR, // temporary, used to add clarity to macro calls
		OBJECT,	// { ... }
		LIST, // [ ... ]

		// containers
		MACRO_OPEN,
		LIST_OPEN,
		OBJ_OPEN,
		PAREN_OPEN,
		CONT_CLOSE,

		// separators
		KV_PAIR,
		COMMA_SERIES,

		// temporary
		OPERATOR,  // un-parsed operator

		INVALID,

		BOUND_ID,
	} type;

	Token token;

	// TODO should be std::vector<AST*>
	std::vector<AST> members;

	// value added after the fact
	int64_t num;

	AST():
		type(INVALID), token(Token()), members() {}
	AST(const NodeType type, const Token& token):
			type(type), token(token), members() {}
	AST(const NodeType type, const Token& token, std::vector<AST> children):
			type(type), token(token), members(std::move(children)) {}
	AST(const AST& other) = default;

	// std::variant<std::string, int64_t, double> val;

	[[nodiscard]] std::string type_name() const {
		switch (this->type) {
			case NodeType::STATEMENTS:		return "Statement sequence";
			case NodeType::OPERATION:		return "Operator Expression";
			case NodeType::NUM_LITERAL:		return "Number Literal";
			case NodeType::STR_LITERAL:		return "String Literal";
			case NodeType::IDENTIFIER:		return "Identifier";
			case NodeType::INVOKE:			return "Macro Call";
			case NodeType::MACRO:			return "Macro Literal";
			case NodeType::INDEX:			return "List Index Request";
			case NodeType::OBJECT:			return "Object Literal";
			case NodeType::LIST:			return "List Literal";
			case NodeType::MACRO_OPEN:		return "Macro opening";
			case NodeType::LIST_OPEN:		return "List opening";
			case NodeType::PAREN_OPEN:		return "Opening Parenthesis";
			case NodeType::OBJ_OPEN:		return "Object opening";
			case NodeType::CONT_CLOSE:		return "Container closing";
			case NodeType::KV_PAIR:			return "Key-Value pair";
			case NodeType::COMMA_SERIES:	return "Comma Series";
			case NodeType::PAREN_EXPR:		return "Parenthesized Expression";
			case NodeType::DECLARATION:		return "Declaration";
			case NodeType::OPERATOR:		return "Operator";
			case NodeType::INVALID:			return "Invalid item";
			default:						return std::string("unknown")
												+ std::to_string(this->type);
		}
	}
	[[nodiscard]] std::string short_type_name() const noexcept {
		switch (this->type) {
			case NodeType::STATEMENTS:		return "SEMIS";
			case NodeType::OPERATION:		return "OP_EXPR";
			case NodeType::NUM_LITERAL:		return "NUM";
			case NodeType::STR_LITERAL:		return "STR";
			case NodeType::IDENTIFIER:		return "ID";
			case NodeType::INVOKE:			return "INVOKE";
			case NodeType::MACRO:			return "MACRO";
			case NodeType::OBJECT:			return "OBJ";
			case NodeType::INDEX:			return "INDEX";
			case NodeType::LIST:			return "LIST";
			case NodeType::MACRO_OPEN:		return "M_OPEN";
			case NodeType::LIST_OPEN:		return "L_OPEN";
			case NodeType::PAREN_OPEN:		return "P_OPEN";
			case NodeType::OBJ_OPEN:		return "O_OPEN";
			case NodeType::CONT_CLOSE:		return "C_CLOSE";
			case NodeType::KV_PAIR:			return "KV_PAIR";
			case NodeType::COMMA_SERIES:	return "COMMA";
			case NodeType::PAREN_EXPR:		return "P_EXPR";
			case NodeType::DECLARATION:		return "DECL";
			case NodeType::OPERATOR:		return "OP";
			case NodeType::INVALID:			return "INVALID";
			default:						return "?:" + std::to_string(this->type);
		}
	}
};

class SyntaxError {
public:
	Token token;
	std::string msg;

	SyntaxError(Token _token, std::string _message):
		token(std::move(_token)), msg(std::move(_message))
	{ }
};

//
AST parse(const std::vector<Token>& tokens);

// Lisp representation
std::string debug_AST(const AST& tree);

#endif //SCL_PARSE_HPP
