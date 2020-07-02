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

	Value* ref = l.deref();
	// todo typerror/nullptr exception
	if (ref == nullptr)
		return;
	*ref = r;

}




namespace VM_ops {
	// change value
	VMOperator single_equals{"change value operator (=)", change_value};

}