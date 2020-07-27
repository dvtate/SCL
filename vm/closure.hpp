//
// Created by tate on 17-05-20.
//

#ifndef DLANG_CLOSURE_HPP
#define DLANG_CLOSURE_HPP

#include <cinttypes>
#include <unordered_map>
#include <memory>
#include <vector>

#include "bc/bc.hpp"
#include "value.hpp"

class Frame;

class Closure {
public:

	int64_t i_id;
	int64_t o_id;

	// captured identifiers
	std::unordered_map<int64_t, Value*> vars;

	// body points to implementation defined by relevant closureDef
	std::vector<BCInstr>* body;

	// rn this only shows if they came from same literal...
	bool operator==(const Closure& other) const {
		return this->body == other.body;
	}
};

namespace GC {
	/// GC mark
	inline void mark(Closure& c) {
		for (auto& p : c.vars)
			mark(p.second);
	}
}
#endif //DLANG_CLOSURE_HPP