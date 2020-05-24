//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>

#include "handle.hpp"
#include "closure.hpp"


// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction() = default;
	virtual void operator()(Frame& f) = 0;
};

class Value {
public:
	using int_t = int64_t;
	using float_t = double;
	using empty_t = char;


	std::variant<std::monostate, float_t, int_t, std::string, Handle<Value>, Closure, Handle<NativeFunction>, empty_t> v;

	enum VType {
		FLOAT,		// float
		INT,		// int
		STR,		// string
		REF,		// reference	Handle<>
		LAM,		//	lambda		Closure
		EMPTY,		// empty		no v
		N_FN,		// native function
	};

	static VType type(int v) {
		// empty on error
		if (v == std::variant_npos)
			return VType::EMPTY;

		const static VType t[] {
			VType::EMPTY,
			VType::EMPTY,
			VType::FLOAT,
			VType::INT,
			VType::STR,
			VType::REF,
			VType::LAM,
			VType::N_FN,
			VType::EMPTY,
		};

		return t[v-1];
	}
	VType type() const {
		return Value::type(v.index());
	}

	Value(){};
	explicit Value(float_t in): 			v(in) {}
	explicit Value(const std::string& in): 		v(in) {}
	explicit Value(int_t in): 				v(in) {}
	explicit Value(const Handle<Value>& in): 		v(in) {}
	explicit Value(const Closure& in): 			v(in) {}
	explicit Value(const Handle<NativeFunction>& in):		v(in) {}
	Value(const Value& other):				v(other.v) {}
};


#endif //DLANG_VALUE_HPP
