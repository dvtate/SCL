//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>

#include "handle.hpp"
#include "closure.hpp"

class Value {
public:
	using int_t = int64_t;
	using float_t = double;

	std::variant<float_t, int_t, Handle<Value>, Closure> v;
	enum VType {
		FLOAT,
		INT,
		STR,
		REF,
		LAM,
		EMPTY
	} type;

	Value(): type(VType::EMPTY) {}
	explicit Value(float_t in):
		type(VType::FLOAT),		v(in) {}
	explicit Value(std::string in):
		type(VType::STR),	v(in) {}
	explicit Value(int_t in):
		type(VType::INT), 	v(in) {}
	explicit Value(Handle<Value> in):
		type(VType::REF),		v(in) {}
	explicit Value(Closure in):
		type(VType::LAM),		v(in) {}

	Value(const Value& other) = default;
};


#endif //DLANG_VALUE_HPP
