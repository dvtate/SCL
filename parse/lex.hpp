//
// Created by tate on 27-04-20.
//

#ifndef SCL_LEX_HPP
#define SCL_LEX_HPP

#include <string>
#include <vector>

#include "../debug.hpp"

// Lexical Tokens
class Token {
public:
	enum t {
		OPERATOR,
		IDENTIFIER,
		STRING,
		NUMBER,
		END,
		ERROR,
		EMPTY,
	} type;
	std::string token;

	// character index of the token in the scanned file
	unsigned long long int pos;

	// File from which the token was sourced
	const char* file;

	Token(const t type, std::string token, const char* file = nullptr, unsigned long long int pos = 0):
			type(type), token(std::move(token)), pos(pos), file(file) {}
	Token() = default;
	Token(const Token& other) = default;
};

// Lexer
std::vector<Token> tokenize_stream(std::istream& in, const char* file_name);

#endif //SCL_LEX_HPP
