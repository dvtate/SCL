//
// Created by tate on 22-05-20.
//
#include <iostream>

#include "../../debug.hpp"
#include "exec_bc_instr.hpp"
#include "../vm.hpp"
#include "../operators/operators.hpp"
#include "../operators/math.hpp"

static void invoke(Frame& f) {

	Value v = f.eval_stack.back();
	f.eval_stack.pop_back();
	if (std::holds_alternative<Value::ref_t>(v.v)) {
		auto& ref = std::get<Value::ref_t>(v.v);
		Value* p = ref.get_ptr()->get_ptr();
		if (!p) {
			std::cout << "Cant invoke null reference...\n";
			return; // TODO: type-error, null-pointer, etc.
		}
		v = Value(*p);
	}

	// call native function
	if (std::holds_alternative<Value::n_fn_t>(v.v)) {
		// std::cout <<"invoke native..\n";
		(*std::get<Handle<NativeFunction>>(v.v).ptr)(f);

	// call lambda sync
	} else if (std::holds_alternative<Value::lam_t>(v.v)) {
		Closure c = std::get<Value::lam_t>(v.v);
//		Frame sync_scope(f.rt, );
	} else {
//		std::cout <<"invoke unknown type...\n";
		// TODO: type-error
	}

}


void exec_bc_instr(Frame& f, BCInstr cmd) {
	DLANG_DEBUG_MSG(cmd.repr() <<std::endl);

	switch (cmd.instr) {
		// push id ref onto stack
		case BCInstr::OPCode::USE_ID:
			f.eval_stack.emplace_back(Value(
					f.closure.vars[cmd.i]));
			return;

		// builtin operator
		case BCInstr::OPCode::BUILTIN_OP: {
			builtin_operators[cmd.i]->act(f);
			return;
		}
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
				ClosureDef &cd = std::get<ClosureDef>(lit.v);

				Closure nc{};

				// capture lexical vars
				// TODO: this is prolly expensive
				for (int64_t cid : cd.capture_ids)
					nc.vars[cid] = f.closure.vars[cid];

				nc.body = &cd.body;


				// todo: implement
			}
			break;
		}

		case BCInstr::OPCode::VAL_EMPTY:
			f.eval_stack.emplace_back(Value());
		default:
//			std::cout <<"unknown bc instr...\n";
			break;
	}
}