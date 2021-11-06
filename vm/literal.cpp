//
// Created by tate on 17-05-20.
//

#include <iostream>
#include "literal.hpp"
#include "value.hpp"


Literal::Literal(const std::string& str, bool is_json) {
	// TODO: is_json determines how to parse...
	(void) is_json;

//	std::cout <<std::boolalpha <<std::holds_alternative<Value::str_t>(Value(str).v) <<std::endl;
	this->v = Value(str);
}