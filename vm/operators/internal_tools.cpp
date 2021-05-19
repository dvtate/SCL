//
// Created by tate on 31-05-20.
//

#include "../value.hpp"
#include "../lambda_return.hpp"
#include "../error.hpp"

#include "internal_tools.hpp"


namespace vm_util {

	void invoke_value_sync(Frame& f, Value& v, bool uncallable) {

		switch (v.type()) {
			case ValueTypes::VType::N_FN:
				// std::cout <<"invoke native..\n";
				(*std::get<ValueTypes::n_fn_ref>(v.v))(f);
				return;
			case ValueTypes::VType::LAM: {
				// Bind input
				Closure c = *std::get<ValueTypes::lam_ref>(v.v);
				if (f.eval_stack.back().type() != ValueTypes::VType::REF) {
					c.vars[c.i_id] = ::new(GC::alloc<Value>()) Value(f.eval_stack.back());
				} else {
					c.vars[c.i_id] = std::get<ValueTypes::ref_t>(f.eval_stack.back().v);
				}
				f.eval_stack.pop_back();

				// Bind output
				c.vars[c.o_id] = ::new(GC::alloc<Value>()) Value((NativeFunction*)
					::new(GC::alloc<LambdaReturnNativeFn>()) LambdaReturnNativeFn(f));

				//
				f.rt->running->stack.emplace_back(new Frame(f.rt, c));
				return;
			};

			default:
				if (uncallable) {
					f.rt->running->stack.back()->eval_stack.back() = v;
				} else {
					f.rt->running->throw_error(gen_error_object("TypeError",
            			std::string("Cannot call value of type") + v.type_name() , f));
				}
		}
	}

}