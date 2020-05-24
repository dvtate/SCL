//
// Created by tate on 17-05-20.
//


#include "literal.hpp"
#include "value.hpp"


Literal::Literal(const std::string& str, bool is_json) {
	// TODO: is_json determines how to parse...
	this->v = std::variant<ClosureDef, Value>(Value(str));
}