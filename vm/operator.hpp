//
// Created by tate on 30-04-20.
//

#ifndef DLANG_OPERATOR_HPP
#define DLANG_OPERATOR_HPP

#include <string>

class Operator {
public:
	std::string name;
	signed char stack_input;
	bool to_arr;
};


#endif //DLANG_OPERATOR_HPP
