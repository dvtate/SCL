//
// Created by tate on 17-05-20.
//

#ifndef DLANG_CLOSURE_HPP
#define DLANG_CLOSURE_HPP

#include <cinttypes>
#include <unordered_map>
#include <memory>

#include "handle.hpp"
#include "literal.hpp"
#include "value.hpp"


// command that falls within a
class BCInstr {
public:
	using OPcode = Command::OPCode;
	OPcode instr;
	union {
		int64_t i;
		double v;
	};
};

class Frame;
class Value;

class Closure {
public:
	static uint64_t _uid;
	uint64_t id;

	// captured identifiers
	std::unordered_map<int64_t, Handle<Value>> vars;
	std::vector<BCInstr> impl;

	std::variant<std::vector<BCInstr>> impl;
	enum ImplType { NATIVE, USER } type;

	Closure(ClosureDef c, Closure parent);
	Closure(ClosureDef c, std::vector<std::string> argv)
};

#endif //DLANG_CLOSURE_HPP