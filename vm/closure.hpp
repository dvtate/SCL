//
// Created by tate on 17-05-20.
//

#ifndef DLANG_CLOSURE_HPP
#define DLANG_CLOSURE_HPP

#include <cinttypes>
#include <unordered_map>
#include <memory>

#include "handle.hpp"
#include "value.hpp"

class Closure {
public:
	static uint64_t uid;
	uint64_t id;


	// captured identifiers
	std::unordered_map<int64_t, std::shared_ptr<Handle<Value>>> id_stack;

};


#endif //DLANG_CLOSURE_HPP
