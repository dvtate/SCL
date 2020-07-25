//
// Created by tate on 07-06-20.
//

#include "logic.hpp"
#include "../vm.hpp"


namespace VM_ops {
	void not_act(Frame& f) {
		f.eval_stack.back() = Value(!f.eval_stack.back().truthy());
	}

	VMOperator logical_not{"Logical not (!)", not_act};

}