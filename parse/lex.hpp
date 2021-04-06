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


	Token(const t type, std::string&& token):
			type(type), token(token), pos(0) {}
	Token(const t type, std::string&& token, unsigned long long int pos):
			type(type), token(token), pos(pos) {}
	Token() = default;
	Token(const Token& other) = default;
};

// Lexer
std::vector<Token> tokenize_stream(std::istream& in);

#endif //SCL_LEX_HPP
