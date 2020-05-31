//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>

#include "gc/handle.hpp"
#include "closure.hpp"



// hack to figure out what I put in std::variant
template <typename>
	struct tag { }; // <== this one IS literal
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
	using empty_t	= std::monostate;
	using int_t 	= int64_t;
	using float_t 	= double;
	using str_t 	= std::string;
	using ref_t 	= Handle<Handle<Value>>;
	using lam_t 	= Closure;
	using n_fn_t 	= Handle<NativeFunction>;
	using bool_t 	= bool;
	using list_t	= std::vector<Value>;

	using variant_t = std::variant<empty_t, float_t, int_t, str_t, ref_t, lam_t, n_fn_t>;
	variant_t v;

	enum VType {
		EMPTY = 0,
		FLOAT = 1,
		INT = 2,
		STR = 3,
		REF = 4,
		LAM = 5,
		N_FN = 6
	};

	Value(){};
	explicit Value(float_t in): 			v(in) {}
	explicit Value(const str_t& in): 		v(in) {}
	explicit Value(int_t in): 				v(in) {}
	explicit Value(const ref_t& in): 		v(in) {}
	explicit Value(const lam_t& in): 			v(in) {}
	explicit Value(const n_fn_t& in):		v(in) {}
	Value(const Value& other):				v(other.v) {}

	inline VType type(){
		return (VType) this->v.index();
	}

};


#endif //DLANG_VALUE_HPP
