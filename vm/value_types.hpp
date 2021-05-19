//
// Created by tate on 25-07-20.
//

#ifndef SCL_VALUE_TYPES_HPP
#define SCL_VALUE_TYPES_HPP

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
	virtual ~NativeFunction() = default;
	// Invoke function
	virtual void operator()(Frame& f) = 0;
	virtual void mark() = 0;
};

namespace ValueTypes {
	using empty_t	= std::monostate;

	// TODO add bigint type and replace int64's with int8's?
	using int_t 	= int64_t;

	using float_t 	= double;

	using str_t 	= std::string;
//	using str_ref	= str_t*; // might as well use fuckin C-string

	// TODO remove ref_t it's failed idea that slows things down for no reason
	// If user wants to pass-by reference they can use objects or arrays
	using ref_t		= Value*;

	// TODO rename *_ref typedecls?

	using lam_t 	= Closure*;
	using lam_ref 	= lam_t;
	using n_fn_t 	= NativeFunction*;
	using n_fn_ref 	= n_fn_t;

	using list_t	= std::vector<Value>;
	using list_ref	= list_t*;

	// TODO need to make custom object type that supports
	// Definitely: pre-hashed keys, selecting values by index, preservation of order
	// Maybe: prototypal inheritance?
	using obj_t		= tsl::ordered_map<std::string, Value>;
	using obj_ref	= obj_t*;

	// This is a stub
	using bool_t 	= ValueTypes::int_t;

	// The only datatype for this lang
	using variant_t = std::variant<empty_t, float_t, int_t, str_t, lam_t, n_fn_t, obj_ref, list_ref, ref_t>;

	// Alligned with variant_t index
	enum class VType {
		EMPTY = 0,
		FLOAT = 1,
		INT = 2,
		STR = 3,
		LAM = 4,
		N_FN = 5,
		OBJ = 6,
		LIST = 7,
		REF = 8,
	};
}

#endif //SCL_VALUE_TYPES_HPP
