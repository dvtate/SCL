//
// Created by tate on 17-05-20.
//

#ifndef DLANG_LITERAL_HPP
#define DLANG_LITERAL_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>

#include "closure.hpp"
#include "../compile/command.hpp"

class ClosureDef {
public:

	// command that falls within a
	class Instr {
	public:
		using OPcode = Command::OPCode;
		OPcode instr;
		union {
			int64_t i;
			double v;
		};
	};

	// all lexical ids used by this closure and it's nested members
	std::vector<int64_t> capture_ids;

	// identifiers declared in scope of this closure
	std::vector<int64_t> decl_ids;

	// instruction code
	std::vector<Instr> body;

};

class Literal {
public:
	std::variant<ClosureDef, Value> v;
	enum Ltype {
		LAM, VAL
	} type;

	explicit Literal(ClosureDef c): v(c), type(Ltype::LAM) {}
	Literal(Value v): v(v), type(Ltype::VAL) {}

	Handle<Value> make_instance();
};


#endif //DLANG_LITERAL_HPP