//
// Created by tate on 31-05-20.
//

#include "internal_tools.hpp"

#include "../value.hpp"
#include "../lambda_return.hpp"


namespace vm_util {

	void invoke_value_sync(Frame &f, Value& v, bool uncallable) {
		auto vt = v.type();
		if (vt == Value::VType::REF) {
			Value* p = v.deref();
			if (p == nullptr) {
				if (uncallable)
					f.eval_stack.emplace_back(v);
				return; // Error: cannot invoke null
			} else {
				v = *p;
			}
			vt = v.type();
		}

		if (vt == Value::VType::N_FN) {
			// std::cout <<"invoke native..\n";
			(*std::get<ValueTypes::n_fn_ref>(v.v))(f);
			return;
		}

		if (vt == Value::VType::LAM) {
			auto c = *std::get<ValueTypes::lam_ref>(v.v);

			// Pass by reference / value
			if (f.eval_stack.back().type() == Value::VType::REF)
				c.vars[c.i_id] = std::get<Value::ref_t>(f.eval_stack.back().v);
			else
				c.vars[c.i_id] = ::new(GC::alloc<Value>()) Value(f.eval_stack.back());
			std::cout <<"input: " <<c.vars[c.i_id]->to_string() <<std::endl;
			f.eval_stack.pop_back();

			c.vars[c.o_id] = ::new(GC::alloc<Value>()) Value((NativeFunction*)
					::new(GC::alloc<LambdaReturnNativeFn>()) LambdaReturnNativeFn(f));

			f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, c));

			return;
		}

		if (uncallable) {
			f.rt->running->back()->eval_stack.emplace_back(v);
		} else {
			// trigger TypeError
		}

	}

}