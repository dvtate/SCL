
// Created by tate on 17-03-20.
//

#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <istream>
#include <iostream>

#include "lex.hpp"


static inline constexpr bool strcont(const char* s, const char c) {
	while (*s && *s != c)
		s++;
	return *s;
}
template <class T>
static inline bool quote_escaped(const T& buff, size_t offset) {
	unsigned char bs = 0;
	while (buff[offset--] == '\\' /* && offset */)
		bs++;
	return bs % 2;
}


template <class T, typename F>
static inline bool end_str(T& buff, size_t& i, const F read, const char start_c = '"') {

	do {
		while (++i < buff.size()) {
			if (buff[i] == start_c && !quote_escaped(buff, i)) {
				return true;
			}
		}
		// didnt find end of string... fetch next line...
		read();
	} while (i < buff.size());
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
			if (buff[i] == '/' && buff[i] - 1 == '*')
				return true;
		}
		// didn't find end of multi-comment

		// get next line
		read();

		// if not eof, scan again
	} while (i < buff.size());

	// !! syntax error unterminated multi-comment
	return false;
}

template <class T> static inline bool
end_num(const T& buff, size_t& i) {
	size_t start = i;
	// starting in different base
	if (buff[i] == '0') {
		i++;
		if (i == buff.size())
			return true;
		// hex
		if (buff[i] == 'x' || buff[i] == 'X') {
			i++;
			while (i < buff.size() && strcont("012345789abcdef", buff[i]))
				i++;
			return true;
			// bin
		} else if (buff[i] == 'b' || buff[i] == 'B') {
			i++;
			while (i < buff.size() && (buff[i] == '1' || buff[i] == '0'))
				i++;
			return true;
			// oct
		} else if (strcont("01234567", buff[i])) {

			i++;
			while (i < buff.size() && strcont("01234567", buff[i]))
				i++;

			// dummy started dec number with a zero :thonk:
			if (buff[i] == '8' || buff[i] == '9') {
				i = start;
			} else
				return true;

		} // else it's dec but they stupid for starting with a zero
	}
	// dec

	// check integer part 123*.456e789
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	// end of number
	if (buff[i] != 'e' && buff[i] != '.')
		return true;
	const bool only_int = buff[i] == 'e';
	i++;
	// next digit-sequence
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	if (only_int || buff[i] != 'e')
		return true;

	// exponent number
	while (i < buff.size() && strcont("0123456789", buff[i]))
		i++;

	return true;
}


template <class T>
static inline void end_id(const T& buff, size_t& i) {
	static const char terminators[] = " .:(){}[]/*&|=^%$#@!~+-;?<>,\t\n\r\\";
	while (i < buff.size() && !strcont(terminators, buff[i]))
		i++;
}

template <class T>
static inline std::string stl_substr(const T& buff, size_t start, const size_t end) {
	std::string ret;
	ret.reserve(end-start);
	for (; start < end; start++)
		ret += buff[start];
	return ret;
}

template <class T, class F>
Token get_token(const T& buff, size_t& i, const F read) {
	// skip spaces
	while (i < buff.size() && isspace(buff[i]))
		i++;

	if (i == buff.size())
		return Token( Token::t::END, "" );

	const char c = buff[i];

	if (c == '/') {
		i++;
		// eof
		if (i == buff.size())
			return Token(Token::t::OPERATOR, "/" );

		// line-comment
		if (buff[i] == '/') {
			i++;
			end_line_comment(buff, i);
			return get_token(buff, i, read);
		}

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
	std::deque<char> buff;

	// fetch next line from stream
	const auto read = [&] () {
		std::string tmp;
		std::getline(in, tmp);
		for (auto c : tmp)
			buff.push_back(c);
	};

	std::vector<Token> ret;

	Token t;

	size_t i = 0; // working index for lex
	size_t pos = 0; // used for getting line numbers for error messages

	for (; ;) {
		const size_t prev_i = i;
		t = get_token(buff, i, read);
		t.pos = prev_i;
		pos += i - prev_i;


		if (t.type == Token::t::END) {

			// try to read next line
			buff.clear();
			i = 0;
			read();
			if (buff.empty()) {
				ret.emplace_back(Token(Token::t::OPERATOR, "eof"));
				ret.back().pos = pos;
				return ret;
			}

		} else if (t.type == Token::t::ERROR) {
			return std::vector<Token>({ t });
		} else {
			//std::cout <<"new tok: " <<t.type <<':' <<t.token <<std::endl;
			ret.emplace_back(t);
		}
	}
}


//std::vector<struct Token> tokenize_string(const std::string& in) {
//	// no more input to fetch
//	const auto read = [](){};
//
//}