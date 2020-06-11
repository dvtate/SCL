//
// Created by tate on 22-05-20.
//
#include <iostream>

#include "../../debug.hpp"
#include "exec_bc_instr.hpp"
#include "../vm.hpp"
#include "../operators/operators.hpp"
#include "../operators/math.hpp"
#include "../lambda_return.hpp"
#include "../operators/internal_tools.hpp"


static void invoke(Frame& f) {

	Value v = f.eval_stack.back();
	f.eval_stack.pop_back();

	vm_util::invoke_value_sync(f, v, false);

}


void index(Frame& f) {
	Value* v = f.eval_stack.back().deref();
	if (v == nullptr) {
		std::cout <<"null passed to INDEX[0]!!!\n";
		return;
	}

	uint64_t ind;
	if (std::holds_alternative<Value::int_t>(v->v)) {
		ind = std::get<Value::int_t>(v->v);
	} else if (std::holds_alternative<Value::float_t>(v->v)) {
		ind = (uint64_t) std::get<Value::float_t>(v->v);
	} else {
		std::cout << "INDEX[0] type-error..\n";
		f.eval_stack.pop_back();
		return;
	}

	f.eval_stack.pop_back();
	v = f.eval_stack.back().deref();
	if (v == nullptr) {
		std::cout <<"null passed to INDEX[1]!!!\n";
		return;
	}
	if (std::holds_alternative<Value::list_t>(v->v)) {
		auto l = std::get<Value::list_t>(v->v);
		try {
			f.eval_stack.back() = l.at(ind);
		} catch (...) {
			f.eval_stack.back() = Value();
		}
	} else {
		std::cout <<"INDEX[1] type-error..\n" <<v->to_string() <<std::endl;
	}
}

void make_list(Frame& f, uint32_t n) {
	Value lv(Value::list_t{});
	auto& l = std::get<Value::list_t>(lv.v);

	// take items off stack and put them into lv -> l
	l.reserve(n);
	l.insert(l.begin(), f.eval_stack.end() - n, f.eval_stack.end());
	f.eval_stack.erase(f.eval_stack.end() - n, f.eval_stack.end());
	f.eval_stack.emplace_back(std::move(lv));
}

void exec_bc_instr(Frame& f, BCInstr cmd) {
	DLANG_DEBUG_MSG(cmd.repr() <<std::endl);
	// OPTIMIZE: use jump table

	switch (cmd.instr) {
		// push id ref onto stack
		case BCInstr::OPCode::USE_ID: try {
				f.eval_stack.emplace_back(Value(
						f.closure.vars.at(cmd.i)));
			} catch (...) {
				DLANG_DEBUG_MSG("Undefined var id: " <<cmd.i <<std::endl);
			}
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

		case BCInstr::OPCode::USE_LIT: {
			Literal &lit = f.rt->vm->literals[cmd.i];
			if (lit.v.index() == Literal::Ltype::VAL) {
				f.eval_stack.emplace_back(std::get<Value>(lit.v));
			} else {
				auto& cd = std::get<ClosureDef>(lit.v);

				auto* c = new Closure();

#ifdef DLANG_DEBUG
				// capture lexical vars
				for (const int64_t cid : cd.capture_ids) {
					try {
						c->vars[cid] = f.closure.vars.at(cid);
					} catch (...) {
						DLANG_DEBUG_MSG("USE_LIT: unable to capture id# " <<cid);
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
				c->lit = cmd.i;

				f.eval_stack.emplace_back(Value(Handle<Closure>(c)));
			}
			return;
		}

		case BCInstr::OPCode::VAL_EMPTY:
			f.eval_stack.emplace_back(Value());
			return;
		case BCInstr::OPCode::CLEAR_STACK:
			f.eval_stack.clear();
			return;
		case BCInstr::OPCode::INDEX:
			index(f);
			return;

		// declaring a mutable identifier
		case BCInstr::OPCode::DECL_ID: {
			auto& v = f.closure.vars[cmd.i];
			if (v.type() == Value::VType::EMPTY) // undefined
				v = Value(Handle( new Handle<Value>(new Value())));
			return;
		};


		case BCInstr::OPCode::MK_LIST:
			make_list(f, cmd.i);
			return;

		default:
			DLANG_DEBUG_MSG("... not implemented\n");
			return;
	}
}