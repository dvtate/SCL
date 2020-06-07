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

		auto* c = std::get<Value::lam_t>(v.v).get_ptr();
		Value arg = f.eval_stack.back();
		f.eval_stack.pop_back();

		// pass arg by reference
		if (std::holds_alternative<Value::ref_t>(arg.v))
			c->vars[c->i_id] = arg;
		else  // wasnt given reference, copy value into one
			c->vars[c->i_id] = Value(Handle(new Handle(new Value(arg))));

		f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, *c));

	} else {
//		std::cout <<"invoke unknown type...\n";
		// TODO: type-error
	}

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
		std::cout <<"INDEX[1] type-error..\n";
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

		case BCInstr::OPCode::MK_LIST:
			make_list(f, cmd.i);
			return;

		default:
			DLANG_DEBUG_MSG("... not implemented\n");
			return;
	}
}