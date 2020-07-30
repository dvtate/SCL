//
// Created by tate on 31-05-20.
//

#include "internal_tools.hpp"

#include "../value.hpp"
#include "../lambda_return.hpp"


namespace vm_util {

	void invoke_value_sync(Frame &f, Value& v, bool uncallable) {

		switch (v.type()) {
			case ValueTypes::VType::REF:
				invoke_value_sync(f, *v.deref(), uncallable);
				return;
			case ValueTypes::VType::N_FN:
				// std::cout <<"invoke native..\n";
				(*std::get<ValueTypes::n_fn_ref>(v.v))(f);
				return;
			case ValueTypes::VType::LAM: {
				Closure c = *std::get<ValueTypes::lam_ref>(v.v);
				if (f.eval_stack.back().type() != ValueTypes::VType::REF) {
					c.vars[c.i_id] = ::new(GC::alloc<Value>()) Value(f.eval_stack.back());
				} else {
					std::cout <<"WTF ref arg?" <<std::endl;
					c.vars[c.i_id] = std::get<Value::ref_t>(f.eval_stack.back().v);
				}
				f.eval_stack.pop_back();

				c.vars[c.o_id] = ::new(GC::alloc<Value>()) Value((NativeFunction*)
					 ::new(GC::alloc<LambdaReturnNativeFn>()) LambdaReturnNativeFn(f));

				f.rt->running->stack.emplace_back(new Frame(f.rt, c));
				return;
			};

			default:
				if (uncallable) {
					f.rt->running->stack.back()->eval_stack.back() = v;
				} else {
					// TODO type-error
				}
		}
	}

}