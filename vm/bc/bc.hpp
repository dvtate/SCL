//
// Created by tate on 24-05-20.
//

#ifndef SCL_BC_HPP
#define SCL_BC_HPP

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

	std::string repr() {
		std::string ret = Command::cmd_name(this->instr);
		auto at = Command::arg_type(this->instr);
		if (at == Command::ArgType::NO_ARG)
			return ret;
		if (at == Command::ArgType::INT16 || at == Command::ArgType::INT32 || at == Command::ArgType::INT64)
			return ret + " " + std::to_string(i);
		else if (at == Command::ArgType::FLOAT)
			return ret + " " + std::to_string(v);

		return ret + "<missing/unk arg>";
	}

	inline static std::string repr(const OPCode oc) {
		return Command::cmd_name(oc);
	}
};

#endif //SCL_BC_HPP
