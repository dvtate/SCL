//
// Created by tate on 24-05-20.
//

#ifndef DLANG_BC_HPP
#define DLANG_BC_HPP

#include "../../compile/command.hpp"

// command that falls within a
class BCInstr {
public:
	using OPCode = Command::OPCode;
	OPCode instr;
	union {
		int64_t i;
		double v;
	};
	BCInstr() = default;
	BCInstr(OPCode cmd, int64_t i): instr(cmd), i(i) {}
	BCInstr(OPCode cmd, double f): instr(cmd), v(f) {}

};

#endif //DLANG_BC_HPP
