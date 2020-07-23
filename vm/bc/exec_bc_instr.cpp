//
// Created by tate on 22-05-20.
//
#include <iostream>

#include "../../debug.hpp"
#include "exec_bc_instr.hpp"
#include "../vm.hpp"
#include "../operators/operators.hpp"
#include "../operators/internal_tools.hpp"

// (())
static void invoke(Frame& f) {
	Value v = f.eval_stack.back();
	f.eval_stack.pop_back();

	vm_util::invoke_value_sync(f, v, false);
}

// ([])
void index(Frame& f) {
	Value* v = f.eval_stack.back().deref();
	if (v == nullptr) {
		std::cout <<"null passed to USE_INDEX[0]!!!\n";
		return;
	}

	uint64_t ind;
	if (std::holds_alternative<Value::int_t>(v->v)) {
		ind = std::get<Value::int_t>(v->v);
	} else if (std::holds_alternative<Value::float_t>(v->v)) {
		ind = (uint64_t) std::get<Value::float_t>(v->v);
	} else {
		std::cout << "USE_INDEX[0] type-error..\n";
		f.eval_stack.pop_back();
		return;
	}

	f.eval_stack.pop_back();
	v = f.eval_stack.back().deref();
	if (v == nullptr) {
		std::cout <<"null passed to USE_INDEX[1]!!!\n";
		return;
	}
	if (std::holds_alternative<Value::list_ref>(v->v)) {
		auto l = std::get<Value::list_ref>(v->v);
		try {
			f.eval_stack.back() = l.ptr->at(ind);
		} catch (...) {
			f.eval_stack.back() = Value();
		}
	} else {
		std::cout <<"USE_INDEX[1] type-error..\n" <<v->to_string() <<std::endl;
	}
}

//
void make_list(Frame& f, uint32_t n) {
	Value lv(Handle(new std::vector<Value>()));
	auto& l = std::get<Value::list_ref>(lv.v).ptr;

	// take items off stack and put them into lv -> l
	l->reserve(n);
	l->insert(l->begin(), f.eval_stack.end() - n, f.eval_stack.end());
	f.eval_stack.erase(f.eval_stack.end() - n, f.eval_stack.end());
	f.eval_stack.emplace_back(std::move(lv));
}

static void use_lit(Frame& f, const std::size_t litnum) {
	Literal &lit = f.rt->vm->literals[litnum];
	if (lit.v.index() == Literal::Ltype::VAL) {
		f.eval_stack.emplace_back(std::get<Value>(lit.v));
	} else {
		auto &cd = std::get<ClosureDef>(lit.v);

		auto *c = new Closure();

#ifdef DLANG_DEBUG
		// capture lexical vars
		for (const int64_t cid : cd.capture_ids) {
			try {
				c->vars[cid] = f.closure.vars.at(cid);
			} catch (...) {
				DLANG_DEBUG_MSG("USE_LIT: unable to capture id# " << cid << std::endl);
			}
		}
#else
		// capture lexical vars
		for (const int64_t cid : cd.capture_ids)
			c->vars[cid] = f.closure.vars[cid];
#endif
		// reference body bytecodes
		c->body = &cd.body;

		// label input and output ids
		c->i_id = cd.i_id();
		c->o_id = cd.o_id();

		// literal index for declaring locals
		c->lit = litnum;

		f.eval_stack.emplace_back(Value(Handle<Closure>(c)));
	}
}


void exec_bc_instr(Frame& f, BCInstr cmd) {
	DLANG_DEBUG_MSG(cmd.repr() <<std::endl);
	// OPTIMIZE: use jump table

	switch (cmd.instr) {
		// push id ref onto stack
		case BCInstr::OPCode::USE_ID:
#ifdef DLANG_DEBUG
			try {
				Value* v = f.closure.vars.at(cmd.i).get_ptr();
				if (v) {
					f.eval_stack.emplace_back(*v);
				} else {
					DLANG_DEBUG_MSG("var initialized to null wtf\n");
					exit(1);
				}
			} catch (...) {
				DLANG_DEBUG_MSG("Undefined var id: " <<cmd.i <<std::endl);
			}
#else
		{
			f.eval_stack.emplace_back(*f.closure.vars[cmd.i]->ptr);
		}
#endif
			return;

		// builtin operator
		case BCInstr::OPCode::BUILTIN_OP:
			builtin_operators[cmd.i]->act(f);
			return;

		// primitive numeric lits
		case BCInstr::OPCode::I64_LIT:
			f.eval_stack.emplace_back(cmd.i);
			return;

		case BCInstr::OPCode::F64_LIT:
			f.eval_stack.emplace_back(cmd.v);
			return;

		case BCInstr::OPCode::INVOKE:
			invoke(f);
			return;

		case BCInstr::OPCode::USE_LIT:
			use_lit(f, cmd.i);
			return;


		case BCInstr::OPCode::VAL_EMPTY:
			f.eval_stack.emplace_back(Value());
			return;
		case BCInstr::OPCode::CLEAR_STACK:
			f.eval_stack.clear();
			return;
		case BCInstr::OPCode::USE_INDEX:
			index(f);
			return;

		// declaring a mutable identifier
		case BCInstr::OPCode::DECL_ID: {
			auto& v = f.closure.vars[cmd.i];
			if (v->ptr == nullptr) // undefined
				v.ptr = new Value();
			return;
		};

		case BCInstr::OPCode::SET_ID:
#ifdef DLANG_DEBUG
			try {
				Value* p = f.closure.vars.at(cmd.i).get_ptr();
				if (!p) {
					DLANG_DEBUG_MSG("id " <<cmd.i <<" = null\n");
				}
				*f.closure.vars[cmd.i].get_ptr() = f.eval_stack.back();
			} catch (...) {
				DLANG_DEBUG_MSG("SET_ID(" <<cmd.i <<") failed\n");
			}
#else
		{
			*f.closure.vars[cmd.i]->ptr = f.eval_stack.back();
		}
#endif
			break;

		case BCInstr::OPCode::MK_LIST:
			make_list(f, cmd.i);
			return;

		case BCInstr::OPCode::SET_INDEX: {
			const Value v = f.eval_stack.back();
			f.eval_stack.pop_back();
			const Value ind_v = f.eval_stack.back();
			std::size_t ind;
			switch (ind_v.type()) {
				case Value::VType::INT:
					ind = std::get<Value::int_t>(ind_v.v);
					break;
				case Value::VType::FLOAT:
					ind = (std::size_t) std::get<Value::float_t>(ind_v.v);
					break;
				default:
					std::cout <<"SET_INDEX: type-error\n";
					return;
			}
			if (f.eval_stack.back().type() == Value::VType::LIST) {
				(*std::get<Value::list_ref>(f.eval_stack.back().v).ptr)[ind] = v;
			} else {
				// typerror
			}
		};

		case BCInstr::OPCode::MK_OBJ:
			if (cmd.i == 0) {
				f.eval_stack.emplace_back(Value(Handle(new Value::obj_t())));
			} else {
				std::cerr <<"sry rn only support {} objects";
			}
			return;

		case BCInstr::OPCode::USE_MEM_L:
			try {
				f.eval_stack.back() = (*std::get<Value::obj_ref>(f.eval_stack.back().v).ptr)
					[std::get<Value::str_t>(std::get<Value>(f.rt->vm->literals[cmd.i].v).v)];

			} catch (const std::bad_variant_access& e) {
				// typeerror
			}
			return;
		case BCInstr::OPCode::SET_MEM_L:
		{
			Value& ref = (*std::get<Value::obj_ref>(f.eval_stack.back().v).ptr)
				[std::get<Value::str_t>(std::get<Value>(f.rt->vm->literals[cmd.i].v).v)];
			f.eval_stack.pop_back();
			ref = f.eval_stack.back();
			return;
		}
		default:
			DLANG_DEBUG_MSG("... not implemented" <<cmd.repr() <<std::endl);
			return;
	}
}