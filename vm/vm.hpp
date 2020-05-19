//
// Created by tate on 17-05-20.
//

#ifndef DLANG_VM_HPP
#define DLANG_VM_HPP


#include <vector>

#include "frame.hpp"


class VM {
	std::vector<Frame> active_contexts;
	// todo: should be new Type Called Frozen Frame
	std::unordered_map<int, Frame> idle_contexts;
};


#endif //DLANG_VM_HPP
