//
// Created by tate on 18-05-20.
//

#include "read_bc.hpp"

#include "../compile/command.hpp"

static inline void ignore_until(std::istream& is, Command::OPCode instr) {
	char c;
	is >>c;
	if (c == instr) {
		return;
	}
	is.ignore();
}

std::vector<Literal> read_lit_header(std::istream& is) {

}