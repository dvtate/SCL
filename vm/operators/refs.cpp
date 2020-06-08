//
// Created by tate on 25-05-20.
//
#include <iostream>


#include "refs.hpp"

#include "../value.hpp"
#include "../vm.hpp"


void change_value(Frame& f) {
	Value r = f.eval_stack.back();
	f.eval_stack.pop_back();
	Value l = f.eval_stack.back();

	if (!std::holds_alternative<Value::ref_t>(l.v)) {
		std::cout <<"lhs eq not a ref?\n";
		return; // todo typeError
	}

	// defer value
	if (std::holds_alternative<Value::ref_t>(r.v))
		std::get<Value::ref_t>(l.v).get_ptr()->set_ptr(
				std::get<Value::ref_t>(r.v).get_ptr()->get_ptr());
	else  {
		Value* ref = std::get<Value::ref_t>(l.v).get_ptr()->get_ptr();
		*ref = r;
	}

	// l stays on stack...
}




namespace VM_ops {
	// change value
	VMOperator single_equals{"change value operator (=)", change_value};

}