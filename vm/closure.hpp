//
// Created by tate on 17-05-20.
//

#ifndef DLANG_CLOSURE_HPP
#define DLANG_CLOSURE_HPP

#include <cinttypes>
#include <unordered_map>
#include <memory>
#include <vector>

#include "handle.hpp"
#include "bc.hpp"


class Frame;
class Value;

class Closure {
public:

//	static uint64_t _uid;
//	uint64_t id;

	// captured identifiers
	std::unordered_map<int64_t, Handle<Value>> vars;

	// body points to implementation defined by relevant closureDef
	std::vector<BCInstr>* body;

};

#endif //DLANG_CLOSURE_HPP