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

#include "bc.hpp"
#include "value.hpp"

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
	ClosureDef() = default;
};

class Literal {
public:
	std::variant<ClosureDef, Value> v;
	enum Ltype {
		ERR = -1,
		LAM = 0,
		VAL = 1
	};

	static inline Ltype type(int i) {
		if (i == std::variant_npos)
			return Ltype::ERR;
		return i ? Ltype::VAL : Ltype::LAM;
	}

	inline Ltype type() const {
		return Literal::type(v.index());
	}

	Literal() {}
	explicit Literal(const ClosureDef& c): v(c) {}
	explicit Literal(const std::string& str, bool is_json = false);

};


#endif //DLANG_LITERAL_HPP