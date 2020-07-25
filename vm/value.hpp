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
#include "gc/gc.hpp"


class Closure;
class Value;

// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction(){};
	// Invoke function
	virtual void operator()(Frame& f) = 0;
	//
	virtual void mark() = 0;

};

namespace ValueTypes {
	using empty_t	= std::monostate;
	using int_t 	= int64_t;
	using float_t 	= double;

	using str_t 	= std::string;
//	using str_ref	= str_t*; // might as well use fuckin C-string
	using ref_t		= Value*;

	using lam = Closure;
	using lam_t 	= Closure*;
	using lam_ref 	= lam_t;
	using n_fn_t 	= NativeFunction*;
	using n_fn_ref 	= n_fn_t;

	using list_t	= std::vector<Value>;
	using list_ref	= list_t*;
	using obj_t		= tsl::ordered_map<std::string, Value>;
	using obj_ref	= obj_t*;

	using bool_t 	= ValueTypes::int_t;

	using variant_t = std::variant<
		empty_t, float_t, int_t, str_t,
		ref_t, lam_t, n_fn_t, obj_ref, list_ref>;

	// Alligned with variant_t index
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
}

class Value {
public:
	// TODO remove these typedefs
	using empty_t	= ValueTypes::empty_t;
	using int_t 	= ValueTypes::int_t;
	using float_t 	= ValueTypes::float_t ;

	using str_t 	= ValueTypes::str_t;
	using ref_t		= ValueTypes::ref_t;
	using lam_t 	= ValueTypes::lam_t;
	using n_fn_t 	= ValueTypes::n_fn_t;

	using list_t	= ValueTypes::list_t;
	using list_ref	= ValueTypes::list_ref;
	using obj_ref	= ValueTypes::obj_ref;

	using VType = ValueTypes::VType;

	// only attribute... could simply extend variant_t...
	ValueTypes::variant_t v;

	Value(){};
	explicit Value(ValueTypes::empty_t in): 			v(in) {}
	explicit Value(ValueTypes::float_t in): 			v(in) {}
	explicit Value(const str_t& in): 					v(in) {}
	explicit Value(ValueTypes::int_t in): 				v(in) {}
	explicit Value(const ValueTypes::ref_t& in): 		v(in) {}
	explicit Value(const ValueTypes::lam_t& in): 		v(in) {}
	explicit Value(const ValueTypes::n_fn_t& in):		v(in) {}
	explicit Value(const ValueTypes::list_t& in):
		v(::new(GC::alloc<ValueTypes::list_t>()) ValueTypes::list_t(in)) {}
	explicit Value(const ValueTypes::list_ref& in):		v(in) {}
	explicit Value(const ValueTypes::obj_t& in):
		v(::new(GC::alloc<ValueTypes::obj_t>()) ValueTypes::obj_t(in)) {}
	explicit Value(const ValueTypes::obj_ref& in):		v(in) {}
	explicit Value(const bool in):						v((ValueTypes::int_t) in) {}
	Value(const Value& other) = default;

	inline ValueTypes::VType type() const {
		return (VType) this->v.index();
	}

	bool eq_value(const Value& other) const;
	bool eq_identity(const Value& other) const;
	bool truthy() const;
	inline Value* deref() const {
		return std::holds_alternative<ValueTypes::ref_t>(v)
			? std::get<ValueTypes::ref_t>(v)
			: (Value*) this;
	}

	std::string to_string(bool recursive = false) const;
};

// Tracing
namespace GC {

	void mark(Value& v);
	void mark(Value* v);

	inline void mark(ValueTypes::obj_t& obj) {
		for (auto& p : obj) {
			mark((Value&) p.second);
		}
	}
	inline void mark(ValueTypes::obj_ref obj) {
		if (mark((void*) obj))
			mark(*obj);
	}
	inline void mark(ValueTypes::list_t& obj) {
		for (auto& v : obj) {
			mark(v);
		}
	}
	inline void mark(ValueTypes::list_ref obj) {
		if (mark((void*) obj))
			mark(*obj);
	}

	inline void mark(NativeFunction& fn) {
		fn.mark();
	}
	inline void mark(ValueTypes::n_fn_t fn) {
		if (mark((void*) fn))
			fn->mark();
	}

	// TODO switch to ValueTypes::lam_t
	void mark(Closure& l);
	inline void mark(ValueTypes::lam_ref l) {
		if (mark((void*) l))
			mark(*l);
	}

	inline void mark(Value& v) {
		switch (v.type()) {
			case ValueTypes::VType::LIST:
				mark(std::get<ValueTypes::list_ref>(v.v));
				return;
			case ValueTypes::VType::OBJ:
				mark(std::get<ValueTypes::obj_ref>(v.v));
				return;
			case ValueTypes::VType::LAM:
				mark(std::get<ValueTypes::lam_ref>(v.v));
				return;
			case ValueTypes::VType::N_FN:
				mark(std::get<ValueTypes::n_fn_ref>(v.v));
				return;
			case ValueTypes::VType::REF:
				mark(std::get<ValueTypes::ref_t>(v.v));
				return;
			default:
				return;
		}
	}
	inline void mark(Value* v) {
		if (mark((void*) v))
			mark(*v);
	}
}

#endif //DLANG_VALUE_HPP
