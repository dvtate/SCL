//
// Created by tate on 28-04-20.
//

#include <iostream>

#include "closure.hpp"
#include "value.hpp"


// aka operator == or operator ?=
bool Value::eq_value(const Value& other) const {
	Value* l, * r;

	// de-ref
	if (std::holds_alternative<Value::ref_t>(this->v))
		l = std::get<Value::ref_t>(this->v).get_ptr();
	else
		l = (Value*) this;

	if (std::holds_alternative<Value::ref_t>(other.v))
		r = std::get<Value::ref_t>(other.v).get_ptr();
	else
		r = (Value*) &other;

	// null == null
	if (l == r)
		return true;

	// chk null, type diffs
	if (l == nullptr || r == nullptr || l->type() != r->type())
		return false;

	if (std::holds_alternative<Value::str_t>(l->v))
		return std::get<Value::str_t>(l->v) == std::get<Value::str_t>(r->v);
	if (std::holds_alternative<Value::int_t>(l->v))
		return std::get<Value::int_t>(l->v) == std::get<Value::int_t>(r->v);
	if (std::holds_alternative<Value::float_t>(l->v))
		return std::get<Value::float_t>(l->v) == std::get<Value::float_t>(r->v);

	// could be different instances of same function
	if (std::holds_alternative<Value::lam_t>(l->v))
		return (*std::get<Value::lam_t>(l->v).get_ptr()) == (*std::get<Value::lam_t>(r->v).get_ptr());
	if (std::holds_alternative<Value::empty_t>(l->v))
		return true;
	if (std::holds_alternative<Value::list_t>(l->v)) {
		auto& ll = *std::get<Value::list_t>(l->v).ptr;
		auto& rl = *std::get<Value::list_t>(r->v).ptr;
		if (ll.size() != rl.size())
			return false;
		for (size_t i = 0; i < ll.size(); i++)
			if (!ll[i].eq_value(rl[i]))
				return false;
		return true;
	}

	// native fns
	if (std::holds_alternative<Value::n_fn_t>(l->v))
		return  std::get<Value::n_fn_t>(l->v).get_ptr() ==
				std::get<Value::n_fn_t>(r->v).get_ptr();

	// todo check unhandled type...
	std::cout <<"ERROR: Value::eq_value typerror: " <<l->v.index() <<" | " <<r->v.index() <<std::endl;
}


bool Value::eq_identity(const Value& other) const {
	if (this->type() != other.type())
		return false;
	if (std::holds_alternative<Value::ref_t>(this->v))
		return  std::get<Value::ref_t>(this->v).get_ptr() ==
				std::get<Value::ref_t>(other.v).get_ptr();

	auto t = this->type();

	if (t == Value::VType::STR)
		return std::get<Value::str_t>(this->v) == std::get<Value::str_t>(other.v);
	if (t == Value::VType::INT)
		return std::get<Value::int_t >(this->v) == std::get<Value::int_t>(other.v);
	if (t == Value::VType::FLOAT)
		return std::get<Value::float_t >(this->v) == std::get<Value::float_t>(other.v);
	if (t == Value::VType::EMPTY)
		return true;
	if (t == Value::VType::LIST) {
		const auto& l = *std::get<Value::list_t>(this->v).ptr;
		const auto& r = *std::get<Value::list_t>(other.v).ptr;
		if (l.size() != r.size())
			return false;
		for (size_t i = 0; i < l.size(); i++)
			if (!l[i].eq_identity(r[i]))
				return false;
		return true;
	}
	if (t == Value::VType::N_FN)
		return std::get<Value::n_fn_t>(this->v).get_ptr() == std::get<Value::n_fn_t>(other.v).get_ptr();
	// must be same instance of same function
	if (t == Value::VType::LAM)
		return std::get<Value::lam_t>(this->v).get_ptr() == std::get<Value::lam_t>(other.v).get_ptr();

	// todo check unhandled type
	// todo check unhandled type...
	std::cout <<"ERROR: Value::eq_identity typerror: " <<t <<std::endl;
}


bool Value::truthy() const {

	auto* val = (Value*) this;
	if (std::holds_alternative<Value::ref_t>(this->v))
		val = std::get<Value::ref_t>(this->v).get_ptr();

//	if (std::holds_alternative<Value::bool_t>(val->v))
//		return std::get<Value::bool_t>(val->v);
	if (std::holds_alternative<Value::int_t>(val->v))
		return std::get<Value::int_t>(val->v);
	if (std::holds_alternative<Value::float_t>(val->v))
		return std::get<Value::float_t>(val->v);
	if (std::holds_alternative<Value::str_t>(val->v))
		return !std::get<Value::str_t>(val->v).empty();
	if (std::holds_alternative<Value::list_t>(val->v))
		return !std::get<Value::list_t>(val->v).ptr->empty();
	if (std::holds_alternative<Value::n_fn_t>(val->v) || std::holds_alternative<Value::lam_t>(val->v))
		return true;
	if (std::holds_alternative<Value::empty_t>(val->v))
		return false;


	// todo: check unhandled type
	std::cout <<"ERROR: Value::truthy typerror: " <<val->v.index() <<std::endl;
}

/// Create String representation of value
// @param recursive: include quotes around strings
std::string Value::to_string(bool recursive) const {
	switch (this->type()) {
		case VType::EMPTY:
			return "empty";
		case VType::INT:
			return std::to_string(std::get<Value::int_t>(this->v));
		case VType::FLOAT:
			return std::to_string(std::get<Value::float_t>(this->v));
		case VType::STR:
			return recursive
				? (std::string("\"") + std::get<Value::str_t>(this->v) + "\"")
				: std::get<Value::str_t>(this->v);
		case VType::LAM:
			return "(: ... )";
		case VType::N_FN:
			return "(: native )";
		case VType::REF: {
			auto* p = std::get<Value::ref_t>(this->v).get_ptr();
			return p != nullptr ? p->to_string(false) : "null";
		};
		case VType::LIST: {
			auto& l = *std::get<Value::list_t>(this->v).ptr;
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