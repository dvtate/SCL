//
// Created by tate on 28-04-20.
//

#include <iostream>
#include <sstream>
#include "lex.hpp"
#include "parse.hpp"

// a quick test program to check changes to parsing

int main() {
	for (; ;) {
		std::string inp;
		std::getline(std::cin, inp);
		std::stringstream ss(inp);
		std::vector<Token> toks = tokenize_stream(ss);
		for (Token tok : toks) {
			std::cout <<'\t' <<tok.type <<':' <<tok.pos <<':' <<tok.token <<std::endl;
		}

		try {
			std::cout << debug_AST(parse(toks)) <<std::endl;
		} catch (std::vector<SyntaxError> es) {
			for (SyntaxError e : es)
				std::cout <<"Syntax Error : "<<e.token.pos <<" : " <<e.msg <<std::endl;
		}
	}
}