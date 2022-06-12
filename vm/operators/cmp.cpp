//
// Created by tate on 07-06-20.
//
#include "../value.hpp"
#include "../vm.hpp"
#include "../error.hpp"
#include "cmp.hpp"

namespace VM_ops {

	void check_equality(Frame& f) {
		Value r = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value l = f.eval_stack.back();

		f.eval_stack.back() = Value(l.eq_value(r));
	}

	void check_identity(Frame& f) {
		Value r = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value l = f.eval_stack.back();

		f.eval_stack.back() = Value(l.eq_identity(r));
	}

	void not_check_equality(Frame& f) {
		check_equality(f);
		auto& v = std::get<ValueTypes::bool_t>(f.eval_stack.back().v);
		v = !v;
	}
	void not_check_identity(Frame& f) {
		check_identity(f);
		auto& v = std::get<ValueTypes::bool_t>(f.eval_stack.back().v);
		v = !v;
	}


// template for comparison operators... will have to replace eventually...
#define SCL_CMP_FN_DEF(FNAME, OP) \
	void FNAME(Frame& f) {\
		Value r = f.eval_stack.back();\
		f.eval_stack.pop_back();\
		Value& l = f.eval_stack.back();\
		SCL_DEBUG_MSG("CMP OP: " <<l.to_string() <<#OP <<r.to_string()<<std::endl);\
		\
		SCL_DEBUG_MSG("CMP OP TYPES: " <<(int) l.type() <<#OP <<(int) r.type()<<std::endl);\
	\
		const auto lt = l.type(), rt = r.type();\
		if (lt == ValueTypes::VType::INT) {\
			if (rt == ValueTypes::VType::INT)\
				f.eval_stack.back() = Value((ValueTypes::int_t) \
						(std::get<ValueTypes::int_t>(l.v) OP std::get<ValueTypes::int_t>(r.v)));\
			else if (rt == ValueTypes::VType::FLOAT)\
				f.eval_stack.back() = Value((ValueTypes::int_t) \
						(std::get<ValueTypes::int_t>(l.v) OP std::get<ValueTypes::float_t>(r.v)));\
			else {\
                f.eval_stack.back() = Value();\
                }\
		} else if (lt == ValueTypes::VType::FLOAT) {\
			if (rt == ValueTypes::VType::FLOAT)\
				f.eval_stack.back() = Value((ValueTypes::int_t) \
						std::get<ValueTypes::float_t>(l.v) OP std::get<ValueTypes::float_t>(r.v));\
			else if (rt == ValueTypes::VType::INT)\
				f.eval_stack.back() = Value((ValueTypes::int_t) \
						(std::get<ValueTypes::float_t>(l.v) OP std::get<ValueTypes::int_t>(r.v)));\
			else  {\
                f.eval_stack.back() = Value(); \
			}\
		} else if (lt == ValueTypes::VType::STR && rt == ValueTypes::VType::STR) {\
			f.eval_stack.back() = Value((ValueTypes::int_t)\
					(std::get<ValueTypes::str_t>(l.v) OP std::get<ValueTypes::str_t>(l.v)));\
		} else {\
            f.rt->running->throw_error(gen_error_object("TypeError", \
            	std::string(#OP "Not supported between types ") + l.type_name() + " and " + r.type_name(), f)); \
			return;\
		}\
	}

	// create operators from template
	SCL_CMP_FN_DEF(lt_act, <)
	SCL_CMP_FN_DEF(gt_act, >)
	SCL_CMP_FN_DEF(le_act, <=)
	SCL_CMP_FN_DEF(ge_act, >=)

	/// check equality
	VMOperator dobule_equals{"compare: equality operator (==)", check_equality};

	/// check Identity
	VMOperator triple_equals{"compare: identity operator (===)", check_identity};

	VMOperator lt{"compare: less than (<)", lt_act};
	VMOperator gt{"compare: greater than (>)", gt_act};
	VMOperator le{"compare: less than or equals (<=)", le_act};
	VMOperator ge{"compare: greater than or equals (>=)", ge_act};

	VMOperator not_double_equals{"compares not equal (!=)", not_check_equality};
	VMOperator not_triple_equals{"compares not identical (!==)", not_check_identity};

}
