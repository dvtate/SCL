//
// Created by tate on 23-05-20.
//

#include "math.hpp"

#include "../vm.hpp"


namespace VM_ops {


// add_any_type
	void add_act(Frame& f) {
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

	void sub_act(Frame& f) {
		Value rhs = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value* l = f.eval_stack.back().deref();
		Value* r = rhs.deref();

		const auto lt = l->type(), rt = r->type();
		if (lt == Value::VType::INT) {
			if (rt == Value::VType::INT) {
				f.eval_stack.back() = Value(
						std::get<Value::int_t>(l->v)
						- std::get<Value::int_t>(r->v));
			} else if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(
						std::get<Value::int_t>(l->v)
						- std::get<Value::float_t>(r->v));
			} else { /* todo: typerror */
				f.eval_stack.back() = Value();
			}
		} else if (lt == Value::VType::FLOAT) {
			if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(
						std::get<Value::float_t>(l->v)
						- std::get<Value::float_t>(r->v));
			} else if (rt == Value::VType::INT) {
				f.eval_stack.back() = Value(
						std::get<Value::float_t>(l->v)
						- std::get<Value::int_t>(r->v));
			} else {
				f.eval_stack.back() = Value();
			}
		} else {
			f.eval_stack.back() = Value();
		}
	};

	VMOperator plus{"plus operator (+)", add_act};
	VMOperator minus{"minus operator (-)", sub_act};
};
