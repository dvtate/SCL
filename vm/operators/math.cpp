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

		switch (lhs.type()) {
			case ValueTypes::VType::INT:
				switch (rhs.type()) {
					case ValueTypes::VType::INT:
						std::get<ValueTypes::int_t>(lhs.v) += std::get<ValueTypes::int_t>(rhs.v);
						break;
					case ValueTypes::VType::FLOAT:
						std::get<ValueTypes::int_t>(lhs.v) += (ValueTypes::int_t) std::get<ValueTypes::float_t>(rhs.v);
						break;
					case ValueTypes::VType::STR:
						lhs.v = std::to_string(std::get<ValueTypes::int_t>(lhs.v)) + std::get<ValueTypes::str_t>(rhs.v);
						break;
				}
				break;

			case ValueTypes::VType::STR:
				switch (rhs.type()) {
					case ValueTypes::VType::INT:
						std::get<ValueTypes::str_t>(lhs.v) += std::to_string(std::get<ValueTypes::int_t>(rhs.v));
						break;
					case ValueTypes::VType::FLOAT:
						std::get<ValueTypes::str_t>(lhs.v) += std::to_string(std::get<ValueTypes::float_t>(rhs.v));
						break;
					case ValueTypes::VType::STR:
						std::get<ValueTypes::str_t>(lhs.v) += std::get<ValueTypes::str_t>(rhs.v);
						break;
				}
				break;
			case ValueTypes::VType::FLOAT:
				switch (rhs.type()) {
					case ValueTypes::VType::INT:
						std::get<ValueTypes::float_t>(lhs.v) += std::get<ValueTypes::int_t>(rhs.v);
						break;
					case ValueTypes::VType::FLOAT:
						std::get<ValueTypes::float_t>(lhs.v) += std::get<ValueTypes::int_t>(rhs.v);
						break;
					case ValueTypes::VType::STR:
						lhs.v = std::to_string(std::get<ValueTypes::float_t>(lhs.v)) + std::get<ValueTypes::str_t>(rhs.v);
						break;
				}
				break;
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
				f.eval_stack.back() = Value(std::get<Value::int_t>(l->v) - std::get<Value::int_t>(r->v));
			} else if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(std::get<Value::int_t>(l->v) - std::get<Value::float_t>(r->v));
			} else { /* todo: typerror */
				f.eval_stack.back() = Value();
			}
		} else if (lt == Value::VType::FLOAT) {
			if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(std::get<Value::float_t>(l->v) - std::get<Value::float_t>(r->v));
			} else if (rt == Value::VType::INT) {
				f.eval_stack.back() = Value(std::get<Value::float_t>(l->v) - std::get<Value::int_t>(r->v));
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
