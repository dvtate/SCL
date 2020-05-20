//
// Created by tate on 02-05-20.
//

#ifndef DLANG_FRAME_HPP
#define DLANG_FRAME_HPP

#include <vector>
#include <cstdint>

#include "closure.hpp"

// Execution frame
class Frame {
public:
	static uint64_t _uid;
	uint64_t id;

	std::vector<Closure> call_stack;
	std::vector<std::vector<Value>> eval_stack;
	explicit Frame();
	explicit Frame(const Closure& c): call_stack({ c }) {}

	// load lit

};

#endif //DLANG_FRAME_HPP
