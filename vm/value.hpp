//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>

#include "../lib/tsl/ordered_map.hpp"

#include "gc/handle.hpp"

class Closure;


// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction() = 0;
	// Invoke function
	virtual void operator()(Frame& f) = 0;
	//
	virtual void mark() = 0;
};

class Value {
public:
	using empty_t	= std::monostate;
	using int_t 	= int64_t;
	using float_t 	= double;

	using str_t 	= std::string;
//	using str_ref	= Handle<std::string>;
	using ref_t		= Handle<Value>;
	using lam_t 	= Handle<Closure>;
	using n_fn_t 	= Handle<NativeFunction>;

	using list_t	= std::vector<Value>;
	using list_ref	= Handle<std::vector<Value>>;
	using obj_t		= tsl::ordered_map<std::string, Value>;
	using obj_ref	= Handle<tsl::ordered_map<std::string, Value>>;

	using bool_t 	= Value::int_t;

	using variant_t = std::variant<
			empty_t, float_t, int_t, str_t,
			ref_t, lam_t, n_fn_t, obj_ref, list_ref>;

	// only attribute... could simply extend variant_t...
	variant_t v;

	// Aligned with v.index
	enum class VType {
		EMPTY = 0,
		FLOAT = 1,
		INT = 2,
		STR = 3,
		REF = 4,
		LAM = 5,
		N_FN = 6,
		OBJ = 7,
		LIST = 8,
	};

	Value(){};
	explicit Value(empty_t in): 			v(in) {}
	explicit Value(float_t in): 			v(in) {}
	explicit Value(const str_t& in): 		v(in) {}
	explicit Value(int_t in): 				v(in) {}
	explicit Value(const ref_t& in): 		v(in) {}
	explicit Value(const lam_t& in): 		v(in) {}
	explicit Value(const n_fn_t& in):		v(in) {}
	explicit Value(const list_t& in):		v(list_ref(new list_t(in))) {}
	explicit Value(const list_ref& in):		v(in) {}
	explicit Value(const obj_t& in):		v(obj_ref(new obj_t(in))) {}
	explicit Value(const obj_ref& in):		v(in) {}
	explicit Value(const bool in):			v((int_t) in) {}
	Value(const Value& other) = default;

	inline VType type() const {
		return (VType) this->v.index();
	}

	bool eq_value(const Value& other) const;
	bool eq_identity(const Value& other) const;
	bool truthy() const;
	inline Value* deref() {
		ref_t &receiver = std::get<Value::ref_t>(v);
		return std::holds_alternative<Value::ref_t>(v)
			? receiver->ptr
			: this;
	}

	std::string to_string(bool recursive = false) const;

	// Mark GC managed objects
	void mark() {
		switch (this->type()) {
			case VType::LAM:
				std::get<lam_t>(this->v).mark();
				return;
			case VType::N_FN:
				std::get<n_fn_t>(this->v).mark();
				return;
			case VType::LIST:
				std::get<list_ref>(this->v).mark();
				return;
			case VType::OBJ:
				std::get<obj_ref>(this->v).mark();
				return;
			case VType::REF:
				std::get<ref_t>(this->v).mark();
				return;
			default:
				return;
		}
	}
};


#endif //DLANG_VALUE_HPP
