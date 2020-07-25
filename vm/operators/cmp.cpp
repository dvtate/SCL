//
// Created by tate on 07-06-20.
//

#include "cmp.hpp"
#include "../value.hpp"
#include "../vm.hpp"

namespace VM_ops {

	void check_equality(Frame& f) {
		// TODO: move to Value.equals()
		Value r = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value l = f.eval_stack.back();

		f.eval_stack.back() = Value(l.eq_value(r));
	}

	void check_identity(Frame& f) {
		// TODO: move to Value.equals()
		Value r = f.eval_stack.back();
		f.eval_stack.pop_back();
		Value l = f.eval_stack.back();

		f.eval_stack.back() = Value(l.eq_identity(r));
	}

	void not_check_equality(Frame&f) {
		check_equality(f);
		auto& v = std::get<ValueTypes::bool_t>(f.eval_stack.back().v);
		v = !v;
	}
	void not_check_identity(Frame&f) {
		check_identity(f);
		auto& v = std::get<ValueTypes::bool_t>(f.eval_stack.back().v);
		v = !v;
	}
	


// template for comparison operators... will have to replace eventually...
#define DLANG_CMP_FN_DEF(FNAME, OP) \
	void FNAME(Frame& f) {\
		Value rhs = f.eval_stack.back();\
		f.eval_stack.pop_back();\
		DLANG_DEBUG_MSG("CMP OP: " <<f.eval_stack.back().to_string() <<rhs.to_string()<<std::endl);\
		Value* l = f.eval_stack.back().deref();\
		Value* r = rhs.deref();\
		\
		if (l == nullptr || r == nullptr) {\
			f.eval_stack.back() = Value(false);\
			return;\
		}\
	\
		const auto lt = l->type(), rt = r->type();\
		if (lt == Value::VType::INT) {\
			if (rt == Value::VType::INT)\
				f.eval_stack.back() = Value(\
						std::get<Value::int_t>(l->v) OP std::get<Value::int_t>(r->v));\
			else if (rt == Value::VType::FLOAT)\
				f.eval_stack.back() = Value(\
						std::get<Value::int_t>(l->v) OP std::get<Value::float_t>(r->v));\
			else\
				f.eval_stack.back() = Value();\
		} else if (lt == Value::VType::FLOAT) {\
			if (rt == Value::VType::FLOAT)\
				f.eval_stack.back() = Value(\
						std::get<Value::float_t>(l->v) OP std::get<Value::float_t>(r->v));\
			else if (rt == Value::VType::INT)\
				f.eval_stack.back() = Value(\
						std::get<Value::float_t>(l->v) OP std::get<Value::int_t>(r->v));\
			else \
				f.eval_stack.back() = Value();\
		} else if (lt == Value::VType::STR && rt == Value::VType::STR) {\
			f.eval_stack.back() = Value(\
					std::get<Value::str_t>(l->v) OP std::get<Value::str_t>(l->v));\
		} else {\
			f.eval_stack.back() = Value();\
		}\
	}

	// create operators from template
	DLANG_CMP_FN_DEF(lt_act, <)
	DLANG_CMP_FN_DEF(gt_act, >)
	DLANG_CMP_FN_DEF(le_act, <=)
	DLANG_CMP_FN_DEF(ge_act, >=)

	// check equality
	VMOperator dobule_equals{"compare: equality operator (==)", check_equality};

	// check Identity
	VMOperator triple_equals{"compare: identity operator (===)", check_identity};

	VMOperator lt{"compare: less than (<)", lt_act};
	VMOperator gt{"compare: greater than (>)", gt_act};
	VMOperator le{"compare: less than or equals (<=)", le_act};
	VMOperator ge{"compare: greater than or equals (>=)", ge_act};

	VMOperator not_double_equals{"compares not equal (!=)", not_check_equality};
	VMOperator not_triple_equals{"compares not identical (!==)", not_check_identity};

}
