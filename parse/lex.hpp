//
// Created by tate on 27-04-20.
//

#ifndef DLANG_LEX_HPP
#define DLANG_LEX_HPP

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


	Token(const t _type, std::string&& _token):
			type(_type), token(_token), pos(0) {}
	Token(const t _type, std::string&& _token, unsigned long long int _pos):
			type(_type), token(_token), pos(_pos) {}
	Token() = default;
	Token(const Token& other) = default;
};

// Lexer
std::vector<Token> tokenize_stream(std::istream& in);

#endif //DLANG_LEX_HPP
