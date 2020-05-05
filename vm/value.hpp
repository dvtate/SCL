////
//// Created by tate on 28-04-20.
////
//
//#ifndef DLANG_VALUE_HPP
//#define DLANG_VALUE_HPP
//
//#include <cinttypes>
//#include <variant>
//
//#include "rt_macro.hpp"
//#include "reference.hpp"
//
//
//class Value {
//public:
//	std::variant<double, int64_t, Handle<Value>, RTMacro> v;
//	enum VType {
//		FLOAT,
//		INT64,
//		REF,
//		MACRO,
//		EMPTY
//	} type;
//
//	Value(): type(VType::EMPTY) {}
//	Value(double in): type(VType::FLOAT), v(in) {}
//	Value(int64_t in): type(VType::INT64), v(in) {}
//	Value(Handle<Value> in): type(VType::REF), v(in) {}
//	Value(RTMacro in);
//
//};
//
//
//#endif //DLANG_VALUE_HPP
