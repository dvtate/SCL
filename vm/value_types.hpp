//
// Created by tate on 25-07-20.
//

#ifndef DLANG_VALUE_TYPES_HPP
#define DLANG_VALUE_TYPES_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>

#include "../lib/tsl/ordered_map.hpp"

class Closure;
class Value;

// Native functions acessable to user
class Frame;
class NativeFunction {
public:
	virtual ~NativeFunction(){};
	// Invoke function
	virtual void operator()(Frame& f) = 0;
	virtual void mark() {};
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

#endif //DLANG_VALUE_TYPES_HPP
