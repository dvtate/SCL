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

	if (!std::holds_alternative<Value::ref_t>(l.v)) {
		std::cout <<"lhs eq not a ref?\n";
		return; // todo typeError
	}

	// defer value
	if (std::holds_alternative<Value::ref_t>(r.v))
		std::get<Value::ref_t>(l.v).set_ptr(
				std::get<Value::ref_t>(r.v).get_ptr());


	if (std::holds_alternative<Value::ref_t>(l.v)) {
		Value* ref = std::get<Value::ref_t>(l.v).get_ptr()->get_ptr();
		*ref = r;
	}

}


static void check_equality(Frame& f) {
	// TODO: move to Value.equals()
	Value r = f.eval_stack.back();
	f.eval_stack.pop_back();
	Value l = f.eval_stack.back();

	// NOTE: user must use triple equals to check type+identity
	if (std::holds_alternative<Value::ref_t>(r.v))
		r = *std::get<Value::ref_t>(r.v).get_ptr()->get_ptr();
	if (std::holds_alternative<Value::ref_t>(l.v))
		l = *std::get<Value::ref_t>(l.v).get_ptr()->get_ptr();

	// diff types
	if (l.type() != r.type())
		f.eval_stack.back() = Value((Value::int_t) 0);
	else if (std::holds_alternative<Value::str_t>(l.v))
		f.eval_stack.back() = Value((Value::int_t)
			(std::get<Value::str_t>(l.v) == std::get<Value::str_t>(r.v)));
	else if (std::holds_alternative<Value::int_t>(l.v))
		f.eval_stack.back() = Value((Value::int_t)
			(std::get<Value::int_t>(l.v) == std::get<Value::int_t>(r.v)));
	else if (std::holds_alternative<Value::float_t>(l.v))
		f.eval_stack.back() = Value((Value::int_t)
			(std::get<Value::float_t>(l.v) == std::get<Value::float_t>(r.v)));
	else if (std::holds_alternative<Value::lam_t>())
}


namespace VM_ops {
	// change value
	VMOperator single_equals{"change value operator (=)", change_value};

	// check equality
	VMOperator dobule_equals{"compare equality operator (==)", check_equality};
//
//	// check identity
//	VMOperator triple_equals{};

}