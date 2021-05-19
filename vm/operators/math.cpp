//
// Created by tate on 23-05-20.
//

#include "../vm.hpp"
#include "../error.hpp"

#include "math.hpp"


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
					default:
						f.rt->running->throw_error(gen_error_object("TypeError",
							std::string("Cannot add values with types ") + lhs.type_name() + " and " + rhs.type_name(), f));
						return;
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
					default:
						f.rt->running->throw_error(gen_error_object("TypeError",
							std::string("Cannot add values with types ") + lhs.type_name() + " and " + rhs.type_name(), f));
						return;
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
					default:
						f.rt->running->throw_error(gen_error_object("TypeError",
							std::string("Cannot add values with types ") + lhs.type_name() + " and " + rhs.type_name(), f));
						return;
				}
				break;
			default:
				f.rt->running->throw_error(gen_error_object("TypeError",
					std::string("Cannot add values with types ") + lhs.type_name() + " and " + rhs.type_name(), f));
				return;
		}
	}

	void sub_act(Frame& f) {
		Value r = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value& l = f.eval_stack.back();

		const auto lt = l.type(), rt = r.type();
		if (lt == Value::VType::INT) {
			if (rt == Value::VType::INT) {
				f.eval_stack.back() = Value(std::get<Value::int_t>(l.v) - std::get<Value::int_t>(r.v));
			} else if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(std::get<Value::int_t>(l.v) - std::get<Value::float_t>(r.v));
			} else {
				f.rt->running->throw_error(gen_error_object("TypeError",
					std::string("Cannot subtract values with types ") + l.type_name() + " and " + r.type_name(), f));
			}
		} else if (lt == Value::VType::FLOAT) {
			if (rt == Value::VType::FLOAT) {
				f.eval_stack.back() = Value(std::get<Value::float_t>(l.v) - std::get<Value::float_t>(r.v));
			} else if (rt == Value::VType::INT) {
				f.eval_stack.back() = Value(std::get<Value::float_t>(l.v) - std::get<Value::int_t>(r.v));
			} else {
				f.eval_stack.back() = Value();
			}
		} else {
			f.rt->running->throw_error(gen_error_object("TypeError",
				std::string("Cannot subtract values with types ") + l.type_name() + " and " + r.type_name(), f));
		}
	};

	VMOperator plus{"plus operator (+)", add_act};
	VMOperator minus{"minus operator (-)", sub_act};
};
