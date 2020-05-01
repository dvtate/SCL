//
// Created by tate on 28-04-20.
//

#ifndef DLANG_VALUE_HPP
#define DLANG_VALUE_HPP

#include <cinttypes>
#include <variant>

#include "rt_macro.hpp"
#include "reference.hpp"


class Value {
public:
	std::variant<double, int64_t, Handle<Value>, RTMacro> v;
	enum VType {
		FLOAT,
		INT64,
		REF,
		MACRO,
		EMPTY
	} type;

	Value();
	Value(double);
	Value(int64_t);
	Value(Handle<Value>);
	Value(RTMacro);

	inline bool isEmpty() const noexcept {
		return type == VType::EMPTY;
	}
	inline bool isInt() const noexcept {
		return type == VType::EMPTY;
	}


};


#endif //DLANG_VALUE_HPP
