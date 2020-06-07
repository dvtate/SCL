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

#include "bc/bc.hpp"
#include "value.hpp"

class ClosureDef {
public:

	// all outside, lexical ids used by this closure and it's nested members
	std::vector<int64_t> capture_ids;

	// identifiers declared in scope of this closure (sorted)
	std::vector<int64_t> decl_ids;

	// instruction code
	std::vector<BCInstr> body;

	ClosureDef(
			std::vector<int64_t> capture_ids,
			std::vector<int64_t> decl_ids,
			std::vector<BCInstr> body):
		capture_ids(std::move(capture_ids)),
		decl_ids(std::move(decl_ids)),
		body(std::move(body))
		{}
	ClosureDef() = default;

	[[nodiscard]] inline int64_t i_id()
		const noexcept { return this->decl_ids[0]; }
	[[nodiscard]] inline int64_t o_id()
		const noexcept { return this->decl_ids[1]; }
};

class Literal {
public:
	std::variant<ClosureDef, Value> v;
	enum Ltype {
		ERR = -1,
		LAM = 0,
		VAL = 1
	};

	static inline Ltype type(size_t i) {
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