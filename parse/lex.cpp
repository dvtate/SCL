
// Created by tate on 17-03-20.
//

#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <istream>
#include <iostream>

#include "lex.hpp"

// TODO this should probably use regex instead

// Returns true if char c is contained in sring s and false otherwise
static inline constexpr bool strcont(const char* s, const char c) {
	while (*s && *s != c)
		s++;
	return *s;
}

// Is the given quote escaped with backslashes?
template <class T>
static inline bool quote_escaped(const T& buff, size_t offset) {
	unsigned char bs = 0;
	while (offset && buff[offset--] == '\\')
		bs++;
	return bs % 2;
}

// Find the end of the string
template <class T, typename F>
static inline bool end_str(T& buff, size_t& i, const F read, const char start_c = '"') {
	// Keep reading file until the end of the string is found
	do {
		// For each char in buffer
		while (++i < buff.size())
			// If the char is the ending character and not escaped we found it
			if (buff[i] == start_c && !quote_escaped(buff, i))
				return true;
		
		// didnt find end of string... fetch next line...
		read();
	} while (!read() && i < buff.size());
	
	// Syntax error unterminated string literal
	return false;
}


// ignore until newline
template <class T>
static inline bool end_line_comment(const T& s, size_t& i) {

	while (i < s.size() && s[i] != '\n')
		i++;
	// NOTE: assuming buffer end is also line end
	return true;
}


// ignore until end of multi-line comment
template <class T, class F> static inline bool
end_multi_comment(const T& buff,
				  size_t& i,
				  const F read)
{
	do {

		while (i < buff.size()) {
			i++;
			if (buff[i] == '/' && buff[i - 1] == '*') {
				i++;
				return true;
			}
		}
		// didn't find end of multi-comment

		// if not eof, scan again
	} while (!read() && i < buff.size());

	// Syntax error unterminated multi-comment
	return false;
}

// Find the end of a number literal
template <class T> static inline bool
end_num(const T& buff, size_t& i) {
	size_t start = i;
	// starting in different base
	if (buff[i] == '0') {
		i++;
		if (i == buff.size())
			return true;
		if (buff[i] == 'x' || buff[i] == 'X') {
			// hex
			i++;
			while (i < buff.size() && strcont("012345789abcdef", buff[i]))
				i++;
			return true;
		
		} else if (buff[i] == 'b' || buff[i] == 'B') {
			// bin
			i++;
			while (i < buff.size() && (buff[i] == '1' || buff[i] == '0'))
				i++;
			return true;
		} else if (strcont("01234567", buff[i])) {
			// oct
			i++;
			while (i < buff.size() && strcont("01234567", buff[i]))
				i++;

			// dummy started dec number with a zero :thonk:
			// try again knowing that it's decimal
			if (buff[i] == '8' || buff[i] == '9') {
				i = start;
			} else
				return true;

		} // else it's dec but they stupid for starting with a zero
	}
	// dec

	// Check integer part 123*.456e789
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	// Integer literal
	if (buff[i] != 'e' && buff[i] != '.')
		return true;

	// Is there a decimal?
	const bool only_int = buff[i] == 'e';
	i++;

	// Next digit-sequence
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	// Prev digit seq was either exponent or decimal place for non 'e' notation number
	if (only_int || buff[i] != 'e')
		return true;

	// Exponent number
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	return true;
}

// Lex identifier token
template <class T>
static inline void end_id(const T& buff, size_t& i) {
	static const char terminators[] = " .:(){}[]/*&|=^%$#@!~+-;?<>,\t\n\r\\";
	while (i < buff.size() && !strcont(terminators, buff[i]))
		i++;
}

// Generic function that returns a string consisting of a section of a stl contianer
template <class T>
static inline std::string stl_substr(const T& buff, size_t start, const size_t end) {
	std::string ret;
	ret.reserve(end-start);
	for (; start < end; start++)
		ret += buff[start];
	return ret;
}

// Tokenizer
template <class T, class F>
Token get_token(const T& buff, size_t& i, const F read) {
	// skip spaces
	while (i < buff.size() && isspace(buff[i]))
		i++;

	// End of buffer
	if (i == buff.size())
		return Token( Token::t::END, "" );

	// Get starting char
	const char c = buff[i];

	// Starts with /
	if (c == '/') {
		// eof
		i++;
		if (i == buff.size())
			return Token(Token::t::OPERATOR, "/" );

		// line-comment
		if (buff[i] == '/') {
			i++;
			end_line_comment(buff, i);
			return get_token(buff, i, read);
		}

		// Multi-line comment
		if (buff[i] == '*') {
			i++;
			if (end_multi_comment(buff, i, read))
				return get_token(buff, i, read);
			else
				return Token( Token::t::ERROR, "unterminated multiline comment (missing */)" );
		}

		// operator that starts with /
		return Token( Token::t::OPERATOR , std::string() + c );
	}

	// string literal
	if (c == '\'' || c == '"') {
		const size_t start = i;
		if (end_str(buff, i, read, c))
			return Token(Token::t::STRING, stl_substr(buff, 1+start, i++) );
		else
			return Token( Token::t::ERROR, (std::string("Unterminated string literal (missing ") + c) + ")" );

	}

	// operator
	const char operators[] = "(){}[].,;:=|&+*-/%#@!?^<>\\";
	if (strcont(operators, c)) {
		i++;
		return Token(Token::t::OPERATOR, std::string() + c);
	}

	// number
	if (strcont("0123456789.", c)) {
		size_t start = i;
		end_num(buff, i);
		return Token(Token::t::NUMBER, stl_substr(buff, start, i));
	}

	// identifier
	size_t start = i;
	end_id(buff, i);
	return Token(Token::t::IDENTIFIER, stl_substr(buff, start, i));
}

// buffered read and tokenize on a line-by-line basis
std::vector<Token> tokenize_stream(std::istream& in) {
	// Temporary read buffer
	std::deque<char> buff;
	
	// Used to track token position
	unsigned long long chars_read = 0;

	// fetch next line from stream
	const auto read = [&] () {
		std::string tmp;
		bool ret = !std::getline(in, tmp);
		chars_read += tmp.size();
		// accept empty lines
		tmp += '\n';
		for (auto c : tmp)
			buff.push_back(c);
		return ret;
	};

	// Return a list of tokens
	std::vector<Token> ret;

	// Current token
	Token t;

	// Working index for lexer
	size_t i = 0;
	
	// Later can be used to get line numbers for error messages
	size_t pos = 0;

	for (; ;) {
		// Get token
		// IIRC weird math here because of behavior when read() is called
		const size_t prev_i = i;
		t = get_token(buff, i, read);
		t.pos = chars_read - (i - prev_i);// pos;
		pos += i > prev_i ? (i - prev_i) : 0;

		if (t.type == Token::t::END) {
			// End of buffer
			
			// try to read next line
			buff.clear();
			i = 0;

			// EOF
			if (read()) {
				ret.emplace_back(Token(Token::t::OPERATOR, "eof"));
				ret.back().pos = pos;
				return ret;
			}

		} else if (t.type == Token::t::ERROR) {
			// Lex failed
			return std::vector<Token>({ t });
		} else {
			// Next token
#ifdef DLANG_DEBUG
			std::cout <<"new tok: " <<t.type <<':' <<t.token <<':' <<t.pos << '/' <<i <<std::endl;
#endif
			ret.emplace_back(t);
		}
	}
}


//std::vector<struct Token> tokenize_string(const std::string& in) {
// TODO std::stringstream
//}
