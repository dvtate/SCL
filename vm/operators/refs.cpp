//
// Created by tate on 25-05-20.
//
#include <iostream>

#include "../value.hpp"
#include "../vm.hpp"
#include "../error.hpp"

#include "refs.hpp"


void change_value(Frame& f) {
	Value r = f.eval_stack.back();
	f.eval_stack.pop_back();
	Value l = f.eval_stack.back();

	Value* ref = l.deref();
	if (ref == nullptr) {
		f.rt->running->throw_error(gen_error_object("TypeError", "Cannot assign value to null reference", f));
		return;
	}
	*ref = r;
}


namespace VM_ops {
	// change value
	VMOperator single_equals{"change value operator (=)", change_value};

}