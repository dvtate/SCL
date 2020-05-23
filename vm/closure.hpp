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

class Frame;
class Value;

class Closure {
public:
	static uint64_t _uid;
	uint64_t id;

	// captured identifiers
	std::unordered_map<int64_t, Handle<Value>> vars;

	// body points to implementation defined in relevant closureDef
	std::vector<BCInstr>* body;
	enum ImplType { NATIVE, USER } type;

	Closure(ClosureDef c, Closure parent);
	Closure(ClosureDef c, std::vector<std::string> argv)
};

#endif //DLANG_CLOSURE_HPP