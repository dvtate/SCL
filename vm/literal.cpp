//
// Created by tate on 17-05-20.
//

#include "literal.hpp"

Handle<Value> Literal::make_instance() {
	if (this->type == Ltype::VAL) {
		return Handle<Value>(new Value(std::get<Value>(this->v)));
	} else if (this->type == Ltype::LAM) {
		// TODO: pull in lexical vars
	}
}