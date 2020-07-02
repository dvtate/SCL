//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>

#include "gc/handle.hpp"

class Closure;


// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction() = default;
	virtual void operator()(Frame& f) = 0;
};

class Value {
public:
	using empty_t	= std::monostate;
	using int_t 	= int64_t;
	using float_t 	= double;
	using str_t 	= std::string;
	using ref_t		= Handle<Value>;
	using lam_t 	= Handle<Closure>;
	using n_fn_t 	= Handle<NativeFunction>;
	using bool_t 	= Value::int_t;
	using list_t	= std::vector<Value>;

	using variant_t = std::variant<
			empty_t, float_t, int_t, str_t,
			ref_t, lam_t, n_fn_t, list_t>;

	// only attribute... could simply extend variant_t...
	variant_t v;

	enum VType {
		EMPTY = 0,
		FLOAT = 1,
		INT = 2,
		STR = 3,
		REF = 4,
		LAM = 5,
		N_FN = 6,
		LIST = 7
	};

	Value(){};
	explicit Value(float_t in): 			v(in) {}
	explicit Value(const str_t& in): 		v(in) {}
	explicit Value(int_t in): 				v(in) {}
	explicit Value(const ref_t& in): 		v(in) {}
	explicit Value(const lam_t& in): 		v(in) {}
	explicit Value(const n_fn_t& in):		v(in) {}
	explicit Value(const list_t& in):		v(in) {}
	explicit Value(const bool in):			v((int_t) in) {}
	Value(const Value& other):				v(other.v) {}

	inline VType type() const {
		return (VType) this->v.index();
	}

	bool eq_value(const Value& other) const;
	bool eq_identity(const Value& other) const;
	bool truthy() const;
	inline Value* deref() {
		return std::holds_alternative<Value::ref_t>(this->v)
			? std::get<Value::ref_t>(this->v).get_ptr()
			: this;
	}

	std::string to_string(bool recursive = false) const;
};


#endif //DLANG_VALUE_HPP
