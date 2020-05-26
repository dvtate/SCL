//
// Created by tate on 25-05-20.
//
#include <iostream>


#include "refs.hpp"

#include "../value.hpp"
#include "../vm.hpp"


static void change_value(Frame& f) {
	Value r = f.eval_stack.back();
	f.eval_stack.pop_back();
	Value l = f.eval_stack.back();



}

static void change_ref(Frame& f) {

}

static void check_equality(Frame& f) {

}

namespace VM_ops {
	// change value
	VMOperator single_equals{"change value operator (=)", change_value};

	// change reference
	VMOperator colon_equals{"change reference operator (:=)", change_ref};

	// check equality
	VMOperator dobule_equals{"compare equality operator (==)", check_equality};
//
//	// check identity
//	VMOperator triple_equals{};

}