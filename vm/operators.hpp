//
// Created by tate on 22-05-20.
//

#ifndef DLANG_OPERATORS_HPP
#define DLANG_OPERATORS_HPP

#include <array>
#include <functional>

class Frame;

class VMOperator {
public:
	const char* name;
	std::function<void(Frame&)> act;
	VMOperator(const char* name, std::function<void(Frame&)> action):
		name(name), act(std::move(action)) {}
};

extern VMOperator builtin_operators[];


#endif //DLANG_OPERATORS_HPP
