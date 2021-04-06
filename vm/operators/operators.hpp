//
// Created by tate on 22-05-20.
//

#ifndef SCL_OPERATORS_HPP
#define SCL_OPERATORS_HPP

#include <vector>
#include <functional>

class Frame;

class VMOperator {
public:
	using name_t = const char*;
	using act_t  = std::function<void(Frame&)>;

	const char* name;
	std::function<void(Frame&)> act;
	VMOperator(const char* name, std::function<void(Frame&)> action):
		name(name), act(std::move(action)) {}
	VMOperator(const char* name, void (*action)(Frame&)):
		name(name), act(action) {}
};

extern std::vector<VMOperator*> builtin_operators;


#endif //SCL_OPERATORS_HPP
