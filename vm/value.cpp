//
// Created by tate on 28-04-20.
//

#include <iostream>
#include <sstream>

#include "closure.hpp"
#include "value.hpp"

/// aka operator == or operator ?=
bool Value::eq_value(const Value& other) const {
	const Value* l = this;
	const Value* r = &other;

	// maybe null == null
	if (l == r)
		return true;

	// chk null, type diffs
	if (l == nullptr || r == nullptr || l->type() != r->type())
		return false;

	if (std::holds_alternative<ValueTypes::str_t>(l->v))
		return std::get<ValueTypes::str_t>(l->v) == std::get<ValueTypes::str_t>(r->v);
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

	auto t = this->type();

	if (t == ValueTypes::VType::STR)
		return std::get<ValueTypes::str_t>(this->v) == std::get<ValueTypes::str_t>(other.v);
	if (t == ValueTypes::VType::INT)
		return std::get<ValueTypes::int_t >(this->v) == std::get<ValueTypes::int_t>(other.v);
	if (t == ValueTypes::VType::FLOAT)
		return std::get<ValueTypes::float_t >(this->v) == std::get<ValueTypes::float_t>(other.v);
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
	if (t == ValueTypes::VType::N_FN) {
		return std::get<ValueTypes::n_fn_t>(v) == std::get<ValueTypes::n_fn_t>(other.v);
	}
	// must be same instance of same function
	if (t == ValueTypes::VType::LAM) {
		return std::get<ValueTypes::lam_ref>(v) == std::get<ValueTypes::lam_ref>(other.v);
	}

	// todo check unhandled type...
	std::cout <<"ERROR: Value::eq_identity typerror: " <<(int)t <<std::endl;
	return false;
}


bool Value::truthy() const {
	auto* val = (Value*) this;
	switch (val->type()) {
		case ValueTypes::VType::INT:
			return std::get<ValueTypes::int_t>(val->v);
		case ValueTypes::VType::FLOAT:
			return (bool) std::get<ValueTypes::float_t>(val->v);
		case ValueTypes::VType::STR:
			return !std::get<ValueTypes::str_t>(val->v).empty();
		case ValueTypes::VType::LIST:
			return !std::get<ValueTypes::list_ref>(val->v)->empty();
		case ValueTypes::VType::LAM: case ValueTypes::VType::N_FN:
			return true;
		case ValueTypes::VType::EMPTY:
			return false;
		default:
			// todo: check unhandled type
			std::cout <<"ERROR: Value::truthy typerror: " <<val->v.index() <<std::endl;
			return false;
	}
//	if (std::holds_alternative<ValueTypes::bool_t>(val->v))
//		return std::get<ValueTypes::bool_t>(val->v);
}

/**
 *  Create String representation of value
 * @param recursive - include quotes around strings or not
 * @return string representation of value
 */
std::string Value::to_string(bool recursive) const {
	switch (this->type()) {
		case ValueTypes::VType::EMPTY:
			return "empty";
		case ValueTypes::VType::INT:
			return std::to_string(std::get<ValueTypes::int_t>(this->v));
		case ValueTypes::VType::FLOAT: {
			std::stringstream ss;
			ss <<std::get<ValueTypes::float_t>(this->v);
			return ss.str();
		}
		case ValueTypes::VType::STR:
			return recursive
				? (std::string("\"") + std::get<ValueTypes::str_t>(this->v) + "\"")
				: std::get<ValueTypes::str_t>(this->v);
		case ValueTypes::VType::LAM:
			return "(: ... )";
		case ValueTypes::VType::N_FN:
			return "(: native )";
		case ValueTypes::VType::LIST: {
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
		case ValueTypes::VType::OBJ: {
			std::string ret = "{";
			auto& o = *std::get<ValueTypes::obj_ref>(this->v);
			if (!o.empty())
				ret += '\n';
			for (auto& p : o) {
				ret += "\"" + p.first + "\" : ";
				ret += p.second.to_string(true) + ",\n";
			}
			ret += '}';
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