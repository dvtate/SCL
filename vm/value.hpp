//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>

#include "handle.hpp"
#include "closure.hpp"



// hack to figure out what I put in std::variant
template <typename> struct tag { }; // <== this one IS literal
template <typename T, typename V>
	struct get_index;
template <typename T, typename... Ts>
	struct get_index<T, std::variant<Ts...>>
		: std::integral_constant<size_t, std::variant<tag<Ts>...>(tag<T>()).index()>
{ };



// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction() = default;
	virtual void operator()(Frame& f) = 0;
};

class Value {
public:
	using empty_t = std::monostate;
	using int_t = int64_t;
	using float_t = double;
	using str_t = std::string;
	using ref_t = Handle<Value>;
	using lam_t = Closure;
	using n_fn_t = Handle<NativeFunction>;
	using bool_t = char;

	std::variant<std::monostate, float_t, int_t, std::string, Handle<Value>, Closure, Handle<NativeFunction>> v;

	enum VType {
		ERR = -1,
		UNDEF = 0,
		FLOAT = 1,		// float
		INT = 2,		// int
		STR = 3,		// string
		REF = 4,		// reference	Handle<>
		LAM = 5,		//	lambda		Closure
		N_FN = 6,		// native function
		EMPTY = 7,		// empty		no v
	};

	static VType type(int v) {
		// empty on error
		if (v == std::variant_npos)
			return VType::EMPTY;

		const static VType t[] {
			VType::ERR,
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
