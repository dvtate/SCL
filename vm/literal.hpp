//
// Created by tate on 17-05-20.
//

#ifndef SCL_LITERAL_HPP
#define SCL_LITERAL_HPP

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

	/// all outside, lexical ids used by this closure and it's nested members
	std::vector<int64_t> capture_ids;

	/// identifiers declared in scope of this closure (sorted)
	std::vector<int64_t> decl_ids;

	/// instruction code
	/// \remark must be freed by owner (or more likely at end of program since few instances)
	std::vector<BCInstr>* body{};

	/// Position in the bytecode that the macro starts at
	std::size_t start_pos{};

	ClosureDef(
			std::vector<int64_t> capture_ids,
			std::vector<int64_t> decl_ids,
			std::vector<BCInstr> body,
			const std::size_t start_pos):
		capture_ids(std::move(capture_ids)),
		decl_ids(std::move(decl_ids)),
		body(new std::vector<BCInstr>(body)),
		start_pos(start_pos)
		{}
	ClosureDef() = default;
	~ClosureDef() = default;

	[[nodiscard]] inline int64_t i_id()
		const noexcept { return this->decl_ids[0]; }
	[[nodiscard]] inline int64_t o_id()
		const noexcept { return this->decl_ids[1]; }

	void destroy() {
		delete(this->body);
	}
};

class Literal {
public:
	std::variant<std::monostate, ClosureDef, Value> v;
	enum Ltype {
		ERR = 0,
		LAM = 1,
		VAL = 2,
	};

	[[nodiscard]] inline Ltype type() const {
		return (Ltype) v.index();
	}

	Literal() = default;
	explicit Literal(const ClosureDef& c): v(c) {}
	explicit Literal(const std::string& str, bool is_json = false);

	void mark() {
		if (this->type() == Ltype::VAL)
			GC::mark(std::get<Value>(v));
	}

	void destroy() {
		if (std::holds_alternative<ClosureDef>(this->v))
			std::get<ClosureDef>(this->v).destroy();
	}
};


#endif //SCL_LITERAL_HPP