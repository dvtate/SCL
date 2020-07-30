//
// Created by tate on 28-04-20.
//

#include <iostream>

#include "closure.hpp"
#include "value.hpp"


// aka operator == or operator ?=
bool Value::eq_value(const Value& other) const {
	Value* l = this->deref(), * r = other.deref();

	// maybe null == null
	if (l == r)
		return true;

	// chk null, type diffs
	if (l == nullptr || r == nullptr || l->type() != r->type())
		return false;

	if (std::holds_alternative<ValueTypes::str_t>(l->v))
		return std::get<ValueTypes::str_t>(l->v) == std::get<Value::str_t>(r->v);
	if (std::holds_alternative<ValueTypes::int_t>(l->v))
		return std::get<ValueTypes::int_t>(l->v) == std::get<ValueTypes::int_t>(r->v);
	if (std::holds_alternative<ValueTypes::float_t>(l->v))
		return std::get<ValueTypes::float_t>(l->v) == std::get<ValueTypes::float_t>(r->v);

	// could be different instances of same function
	if (std::holds_alternative<ValueTypes::lam_t>(l->v)) {
		return (*std::get<ValueTypes::lam_t>(l->v)) == (*std::get<ValueTypes::lam_t>(r->v));
	}
	if (std::holds_alternative<ValueTypes::empty_t>(l->v))
		return true;
	if (std::holds_alternative<ValueTypes::list_ref>(l->v)) {
		auto& ll = *std::get<ValueTypes::list_ref>(l->v);
		auto& rl = *std::get<ValueTypes::list_ref>(r->v);
		if (ll.size() != rl.size())
			return false;
		for (size_t i = 0; i < ll.size(); i++)
			if (!ll[i].eq_value(rl[i]))
				return false;
		return true;
	}

	// native fns
	if (std::holds_alternative<ValueTypes::n_fn_t>(l->v))
		return std::get<ValueTypes::n_fn_t>(l->v) == std::get<ValueTypes::n_fn_t>(r->v);

	// todo check unhandled type...
	std::cout <<"ERROR: Value::eq_value typerror: " <<l->v.index() <<" | " <<r->v.index() <<std::endl;
	return false;
}


bool Value::eq_identity(const Value& other) const {
	if (this->type() != other.type())
		return false;
	if (std::holds_alternative<ValueTypes::ref_t>(this->v)) {
		return std::get<ValueTypes::ref_t>(v) == std::get<ValueTypes::ref_t>(other.v);
	}

	auto t = this->type();

	if (t == Value::VType::STR)
		return std::get<ValueTypes::str_t>(this->v) == std::get<ValueTypes::str_t>(other.v);
	if (t == Value::VType::INT)
		return std::get<ValueTypes::int_t >(this->v) == std::get<ValueTypes::int_t>(other.v);
	if (t == Value::VType::FLOAT)
		return std::get<ValueTypes::float_t >(this->v) == std::get<Value::float_t>(other.v);
	if (t == ValueTypes::VType::EMPTY)
		return true;
	if (t == ValueTypes::VType::LIST) {
		const auto& l = *std::get<ValueTypes::list_ref>(this->v);
		const auto& r = *std::get<ValueTypes::list_ref>(other.v);
		if (l.size() != r.size())
			return false;
		for (size_t i = 0; i < l.size(); i++)
			if (!l[i].eq_identity(r[i]))
				return false;
		return true;
	}
	if (t == Value::VType::N_FN) {
		return std::get<ValueTypes::n_fn_t>(v) == std::get<ValueTypes::n_fn_t>(other.v);
	}
	// must be same instance of same function
	if (t == Value::VType::LAM) {
		return std::get<ValueTypes::lam_ref>(v) == std::get<ValueTypes::lam_ref>(other.v);
	}

	// todo check unhandled type...
	std::cout <<"ERROR: Value::eq_identity typerror: " <<(int)t <<std::endl;
	return false;
}


bool Value::truthy() const {
	auto* val = (Value*) this;
	switch (val->type()) {
		case VType::REF: {
			return std::get<ValueTypes::ref_t>(v)->truthy();
		}
		case ValueTypes::VType::INT:
			return std::get<ValueTypes::int_t>(val->v);
		case ValueTypes::VType::FLOAT:
			return std::get<ValueTypes::float_t>(val->v);
		case ValueTypes::VType::STR:
			return !std::get<ValueTypes::str_t>(val->v).empty();
		case ValueTypes::VType::LIST:
			return !std::get<ValueTypes::list_ref>(val->v)->empty();
		case ValueTypes::VType::LAM: case VType::N_FN:
			return true;
		case ValueTypes::VType::EMPTY:
			return false;
		default:
			// todo: check unhandled type
			std::cout <<"ERROR: Value::truthy typerror: " <<val->v.index() <<std::endl;
			return false;
	}
//	if (std::holds_alternative<Value::bool_t>(val->v))
//		return std::get<Value::bool_t>(val->v);
}

/// Create String representation of value
// @param recursive: include quotes around strings
std::string Value::to_string(bool recursive) const {
	switch (this->type()) {
		case VType::EMPTY:
			return "empty";
		case VType::INT:
			return std::to_string(std::get<ValueTypes::int_t>(this->v));
		case VType::FLOAT:
			return std::to_string(std::get<ValueTypes::float_t>(this->v));
		case VType::STR:
			return recursive
				? (std::string("\"") + std::get<ValueTypes::str_t>(this->v) + "\"")
				: std::get<ValueTypes::str_t>(this->v);
		case VType::LAM:
			return "(: ... )";
		case VType::N_FN:
			return "(: native )";
		case VType::REF: {
			const auto p = std::get<ValueTypes::ref_t>(v);
			return p != nullptr ? p->to_string(false) : "null";
		};
		case VType::LIST: {
			auto& l = *std::get<ValueTypes::list_ref>(this->v);
			std::string ret = "[ ";
			if (!l.empty())
				ret += l[0].to_string(true);
			for (std::size_t i = 1; i < l.size(); i++) {
				ret += ", ";
				ret += l[i].to_string(true);
			}
			ret += " ]";
			return ret;
		}
		default:
			return "unknown";
	}
}

namespace GC {
	void mark(NativeFunction& fn) {
		fn.mark();
	}
}