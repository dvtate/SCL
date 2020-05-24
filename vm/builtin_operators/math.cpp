//
// Created by tate on 23-05-20.
//

#include "math.hpp"

#include "../vm.hpp"

static void add_act(Frame& f) {
//
//		Value rhs = f.eval_stack.back();
//		f.eval_stack.pop_back();
//		Value& lhs = f.eval_stack.back();
//
//
//		// defer references
//		if (rhs.type() == Value::VType::REF) {
//			Value* p = std::get<Handle<Value>>(rhs.v).ptr;
//			if (p == nullptr)
//				return; // TODO: type-error
//			rhs = *p;
//		}
//
//		if (lhs.type() == Value::VType::REF) {
//			Value* p = std::get<Handle<Value>>(lhs.v).ptr;
//			if (p == nullptr)
//				return; // TODO: type-error
//			lhs = *p;
//		}
//
//		auto rhs_type = rhs.type();
//		auto lhs_type = lhs.type();
//		// perform relevant operation
//		if (rhs_type == Value::VType::INT) {
//			Value::int_t& i = std::get<Value::int_t>(rhs.v);
//			if (lhs_type == Value::VType::INT) {
//				std::get<Value::int_t>(lhs.v) += i;
//			} else if (lhs_type == Value::VType::FLOAT) {
//				lhs = Value(std::get<Value::float_t>(lhs.v) + (Value::float_t) i);
//			} else if (lhs_type == Value::VType::STR) {
//				std::get<std::string>(lhs.v) += std::to_string(i);
//			} else {
//				// TODO: type-error
//			}
//
//		} else if (rhs_type == Value::VType::STR) {
//			std::string& s = std::get<std::string>(rhs.v);
//			if (lhs_type == Value::VType::INT) {
//				lhs = Value(std::to_string(std::get<Value::int_t>(lhs.v)) + s);
//			} else if (lhs_type == Value::VType::FLOAT) {
//				lhs = Value(std::to_string(std::get<Value::float_t>(lhs.v)) + s);
//			} else if (lhs_type == Value::VType::STR) {
//				std::get<std::string>(lhs.v) += s;
//			} else {
//				// TODO: type-error
//			}
//		} else if (rhs_type == Value::VType::FLOAT) {
//			Value::float_t &fp = std::get<Value::float_t>(rhs.v);
//			if (lhs_type == Value::VType::INT) {
//				lhs = Value(std::get<Value::int_t>(lhs.v) + fp);
//			} else if (lhs_type == Value::VType::FLOAT) {
//				std::get<Value::float_t>(lhs.v) += fp;
//			} else if (lhs_type == Value::VType::STR) {
//				std::get<std::string>(lhs.v) += std::to_string(fp);
//			} else {
//				// TODO: type-error
//			}
//		} else {
//			// TODO: type-error
//		}
//
//		return;

}

namespace VM_ops {

	auto add = VMOperator("plus operator (+)", add_act);
};
