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
	std::variant<double, int64_t, Handle<Value>, Closure> v;
	enum VType {
		FLOAT,
		INT64,
		STR,
		REF,
		LAM,
		EMPTY
	} type;

	Value(): type(VType::EMPTY) {}
	explicit Value(double in):
		type(VType::FLOAT),		v(in) {}
	explicit Value(std::string in):
		type(VType::STR),	v(in) {}
	explicit Value(int64_t in):
		type(VType::INT64), 	v(in) {}
	explicit Value(Handle<Value> in):
		type(VType::REF),		v(in) {}
	explicit Value(Closure in):
		type(VType::LAM),		v(in) {}

};


#endif //DLANG_VALUE_HPP
