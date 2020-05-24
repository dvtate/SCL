//
// Created by tate on 17-05-20.
//

#ifndef DLANG_LITERAL_HPP
#define DLANG_LITERAL_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>
#include <unordered_set>

#include "../compile/command.hpp"
#include "value.hpp"

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
	BCInstr(OPCode cmd, int64_t f): instr(cmd), v(f) {}

};

class ClosureDef {
public:

	// io ids
	int64_t i_id;
	int64_t o_id;

	// all outside, lexical ids used by this closure and it's nested members
	std::vector<int64_t> capture_ids;

	// identifiers declared in scope of this closure
	std::vector<int64_t> decl_ids;

	// instruction code
	std::vector<BCInstr> body;

	ClosureDef(
			const std::vector<int64_t>& capture_ids,
			const std::vector<int64_t>& decl_ids,
			const std::vector<BCInstr>& body,
			const int64_t i_id, const int64_t o_id):
		capture_ids(capture_ids), decl_ids(decl_ids), body(body), i_id(i_id), o_id(o_id)
		{}
};

class Literal {
public:
	std::variant<ClosureDef, Value> v;
	enum Ltype {
		LAM, VAL
	} type;

	explicit Literal(ClosureDef c): v(c), type(Ltype::LAM) {}
	Literal(Value v): v(v), type(Ltype::VAL) {}

};


#endif //DLANG_LITERAL_HPP