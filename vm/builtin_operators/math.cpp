//
// Created by tate on 23-05-20.
//

#include "math.hpp"

#include "../vm.hpp"


// add_any_type
static void add_act(Frame& f) {
	Value rhs = f.eval_stack.back();
	f.eval_stack.pop_back();
	Value& lhs = f.eval_stack.back();


	// dereference
	if (std::holds_alternative<Value::ref_t>(rhs.v)) {
		Value *p = std::get<Value::ref_t>(rhs.v).get_ptr()->get_ptr();
		if (p == nullptr) return; // TODO: type-error/nullptr exception
		rhs = *p;
	}
	if (std::holds_alternative<Value::ref_t>(lhs.v)) {
		Value *p = std::get<Value::ref_t>(lhs.v).get_ptr()->get_ptr();
		if (p == nullptr) return; // TODO: type-error/nullptr exception

		// this also copies so that we don't mutate referenced value
		lhs = *p;
	}

	// operation based on datatype
	if (std::holds_alternative<Value::int_t>(rhs.v)) {
		auto&& r = std::get<Value::int_t>(rhs.v);

		if (std::holds_alternative<Value::int_t>(lhs.v))
			std::get<Value::int_t>(lhs.v) += r;
		else if (std::holds_alternative<Value::float_t>(lhs.v))
			std::get<Value::float_t>(lhs.v) += (Value::float_t) r;
		else if (std::holds_alternative<Value::str_t>(lhs.v))
			std::get<Value::str_t>(lhs.v) += std::to_string(r);
		else // TODO: type-error
			return;

	} else if (std::holds_alternative<Value::float_t>(rhs.v)) {
		auto&& r = std::get<Value::float_t>(rhs.v);

		if (std::holds_alternative<Value::int_t>(lhs.v))
			lhs.v = (Value::float_t) std::get<Value::int_t>(lhs.v) + r;
		else if (std::holds_alternative<Value::float_t>(lhs.v))
			std::get<Value::float_t>(lhs.v) += r;
		else if (std::holds_alternative<Value::str_t>(lhs.v))
			std::get<Value::str_t>(lhs.v) += std::to_string(r);
		else // TODO: type-error
			return;

	} else if (std::holds_alternative<Value::str_t>(rhs.v)) {
		auto&& r = std::get<Value::str_t>(rhs.v);

		if (std::holds_alternative<Value::int_t>(lhs.v))
			lhs.v = std::to_string(std::get<Value::int_t>(lhs.v)) + r;
		else if (std::holds_alternative<Value::float_t>(lhs.v))
			lhs.v = std::to_string(std::get<Value::float_t>(lhs.v)) + r;
		else if (std::holds_alternative<Value::str_t>(lhs.v))
			std::get<Value::str_t>(lhs.v) += r;
		else // TODO: type-error
			return;

	} else {
		// TODO: type-error
		return;
	}


}

namespace VM_ops {
	VMOperator plus{"plus operator (+)", add_act};
};
