//
// Created by tate on 17-05-20.
//

#include "closure.hpp"
#include "frame.hpp"


uint64_t Closure::_uid = 0;

// constructed from literal
Closure::Closure(ClosureDef c, Frame& f) {
	this->type = ImplType::USER;
	this->vars =
}