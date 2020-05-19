//
// Created by tate on 18-05-20.
//

#include <map>
#include "read_bc.hpp"
#include "literal.hpp"
#include "../compile/command.hpp"

//
static inline void ignore_until(std::istream& is, Command::OPCode instr) {
	char c;
	for (;;) {
		is >>c;
		if (c == instr)
			return;
		is.ignore(8);
	}
}
// convert escaped characters into proper ascii codes
static inline std::string parse_str(std::string in) {
	// TODO: implement (can steal code from ys)
	return in;
}




std::vector<Literal> read_lit_header(std::istream& is) {
	std::vector<Literal> ret;

	char instr;
	is >> instr;

	std::map<int64_t, std::vector<int64_t>> nested_closures;

	while (instr != Command::OPCode::END_LIT_SECTION) {

		if (instr == Command::OPCode::START_LIT_STRING) {
			char c;
			std::string strlit;
			do {
				is >>c;
				if (c)
					strlit += c;
				else
					break;
			} while (c);

			ret.emplace_back(Literal(Value(parse_str(strlit))));
		} else if (instr == Command::OPCode::START_LIT_MACRO){

		}

		// fetch next instruction
		is >> instr;
	}

}