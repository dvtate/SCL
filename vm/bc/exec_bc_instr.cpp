//
// Created by tate on 22-05-20.
//
#include <iostream>

#include "../../debug.hpp"
#include "../vm.hpp"
#include "../operators/operators.hpp"
#include "../operators/internal_tools.hpp"
#include "../error.hpp"
#include "../primitive_methods.hpp"

#include "exec_bc_instr.hpp"


/// Call macro
static inline void invoke(Frame& f) {
	Value v = f.eval_stack.back();
	f.eval_stack.pop_back();
	vm_util::invoke_value_sync(f, v, false);
}

/// [ ... ]
void make_list(Frame& f, uint32_t n) {
	Value lv(f.gc_make<ValueTypes::list_t>());
	auto* l = std::get<ValueTypes::list_ref>(lv.v);

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
		auto* c = f.gc_make<Closure>();

#ifdef SCL_DEBUG
		// capture lexical vars
		for (const int64_t cid : cd.capture_ids) {
			try {
				c->vars[cid] = f.closure.vars.at(cid);
			} catch (...) {
				SCL_DEBUG_MSG("USE_LIT: unable to capture id# " << cid << std::endl);
			}
		}
#else
		// capture lexical vars
		for (const int64_t cid : cd.capture_ids)
			c->vars[cid] = f.closure.vars[cid];
#endif
		// reference body bytecodes
		c->body = cd.body;

		// label input and output ids
		c->i_id = cd.i_id();
		c->o_id = cd.o_id();

		f.eval_stack.emplace_back(c);
	}
}


void exec_bc_instr(Frame& f, BCInstr cmd) {
	SCL_DEBUG_MSG(cmd.repr() << std::endl);
	// OPTIMIZE: use jump table

	switch (cmd.instr) {
		// push id ref onto stack
		case BCInstr::OPCode::USE_ID:
#ifdef SCL_DEBUG
			try {
				Value* v = f.closure.vars.at(cmd.i);
				if (v) {
					f.eval_stack.emplace_back(*v);
				} else {
					SCL_DEBUG_MSG("var initialized to null wtf\n");
					exit(1);
				}
			} catch (...) {
				SCL_DEBUG_MSG("Undefined var id: " << cmd.i << std::endl);
			}
#else
//			std::cout <<"vid:" <<cmd.i
//				<<"\tptr: " <<(void*) f.closure.vars[cmd.i]
//				<< "\tv: " <<f.closure.vars[cmd.i]->to_string() <<std::endl;
			f.eval_stack.emplace_back(*f.closure.vars[cmd.i]);
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
			f.eval_stack.emplace_back();
			return;
		case BCInstr::OPCode::VAL_TRUE:
			f.eval_stack.emplace_back((ValueTypes::int_t) 1);
			return;
		case BCInstr::OPCode::VAL_FALSE:
			f.eval_stack.emplace_back((ValueTypes::int_t) 0);
			return;
		case BCInstr::OPCode::CLEAR_STACK:
			f.eval_stack.clear();
			return;
		case BCInstr::OPCode::USE_INDEX: {
			Value index = f.eval_stack.back();
			f.eval_stack.pop_back();
			Value& v = f.eval_stack.back();
			f.eval_stack.back() = vm_util::index_value(f, v, index);
			return;
		}

		// declaring a mutable identifier
		case BCInstr::OPCode::DECL_ID: {
			auto*& v = f.closure.vars[cmd.i];
			if (v == nullptr) // undefined
				v = f.gc_make<Value>();
			return;
		};

		case BCInstr::OPCode::SET_ID:
#ifdef SCL_DEBUG
			try {
				Value* p = f.closure.vars.at(cmd.i);
				if (!p) {
					SCL_DEBUG_MSG("id " << cmd.i << " = null\n");
				}
				*f.closure.vars[cmd.i] = f.eval_stack.back();
			} catch (...) {
				SCL_DEBUG_MSG("SET_ID(" << cmd.i << ") failed\n");
			}
#else
			*f.closure.vars[cmd.i] = f.eval_stack.back();
#endif
			break;

		case BCInstr::OPCode::MK_LIST:
			make_list(f, cmd.i);
			return;

		case BCInstr::OPCode::SET_INDEX: {
			// TODO bounds checking in debug mode
			const Value v = f.eval_stack.back();
			f.eval_stack.pop_back();
			const Value ind_v = f.eval_stack.back();
			f.eval_stack.pop_back();
			ssize_t ind;
			std::string mem;
			switch (ind_v.type()) {
				case ValueTypes::VType::INT:
					ind = std::get<ValueTypes::int_t>(ind_v.v);
					break;
				case ValueTypes::VType::FLOAT:
					ind = (std::size_t) std::get<ValueTypes::float_t>(ind_v.v);
					break;
				case ValueTypes::VType::STR:
					if (f.eval_stack.back().type() != ValueTypes::VType::OBJ)
						std::cerr <<"SET_INDEX: invalid string indexing for "
							<< (int) f.eval_stack.back().type() <<std::endl;
					mem = std::get<ValueTypes::str_t>(ind_v.v);
					break;
				default:
					f.rt->running->throw_error(gen_error_object(
						"TypeError",
						std::string("Index expected Int, Float or String, received ") + ind_v.type_name(), f));
					std::cerr <<"SET_INDEX: type-error " <<(int) ind_v.type() <<" - " <<ind_v.to_string(true) <<std::endl;
					return;
			}
			switch (f.eval_stack.back().type()) {
				case ValueTypes::VType::LIST:
					(*std::get<ValueTypes::list_ref>(f.eval_stack.back().v))[ind] = v;
					break;
				case ValueTypes::VType::STR:
					try {
						std::get<ValueTypes::str_t>(f.eval_stack.back().v)[ind] =
						        std::get<ValueTypes::str_t>(v.v)[0];
					} catch (...) {}
					break;
				case ValueTypes::VType::OBJ:
					try {
//						std::cout <<f.eval_stack.back().to_string(true) <<"." <<mem <<" = " <<v.to_string(true) <<std::endl;
						(*std::get<ValueTypes::obj_ref>(f.eval_stack.back().v))[mem] = v;
//						std::cout <<f.eval_stack.back().to_string(true) <<std::endl;
					} catch (...) {}
					break;

				default:
					f.rt->running->throw_error(gen_error_object(
						"TypeError",
						std::string("cannot index type - ") + f.eval_stack.back().type_name(),
						f));
					break;
			}
			break;
		};

		case BCInstr::OPCode::MK_OBJ:
			// Make object
			f.eval_stack.emplace_back(f.gc_make<ValueTypes::obj_t>());
			return;

		case BCInstr::OPCode::USE_MEM_L: {
			// Object member getter
			Value& v = f.eval_stack.back();
			const auto& str = std::get<ValueTypes::str_t>(std::get<Value>(f.rt->vm->literals[cmd.i].v).v);
			switch (v.type()) {
				case ValueTypes::VType::OBJ:
					f.eval_stack.back() = (*std::get<ValueTypes::obj_ref>(v.v))
						[str];
					return;
				case ValueTypes::VType::EMPTY:
					f.rt->running->throw_error(gen_error_object(
						"TypeError",
						std::string("cannot accesss property") + str + " of empty",
						f));
					return;
				default:
					f.eval_stack.back() = get_primitive_member(f, f.eval_stack.back(), str);

			}
			return;
		}

		case BCInstr::OPCode::SET_MEM_L:
			// Object member setter
		{
			// Get value
			Value v = std::move(f.eval_stack.back());
			f.eval_stack.pop_back();

			// Set object member
			// TODO pre-hash the literal
			(*std::get<ValueTypes::obj_ref>(f.eval_stack.back().v))
				[std::get<ValueTypes::str_t>(std::get<Value>(f.rt->vm->literals[cmd.i].v).v)]
				= std::move(v);

			// Object stays on the stack
			return;
		}

		case BCInstr::OPCode::VAL_CATCH:
			f.eval_stack.emplace_back((NativeFunction*) f.gc_make<CatchFn>(f));
			break;

		default:
			SCL_DEBUG_MSG("... not implemented" << cmd.repr() << std::endl);
			return;
	}
}