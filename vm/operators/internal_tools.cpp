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
			Value *p = std::get<Value::ref_t>(v.v).get_ptr();
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
			(*std::get<Handle<NativeFunction>>(v.v).get_ptr())(f);
			return;
		}

		if (vt == Value::VType::LAM) {
			auto c = *std::get<Value::lam_t>(v.v).get_ptr();

			// Pass by reference
			if (f.eval_stack.back().type() == Value::VType::REF)
				c.vars[c.i_id].set_ptr(std::get<Value::ref_t>(f.eval_stack.back().v).get_ptr());
			else
				c.vars[c.i_id].set_ptr(new Value(f.eval_stack.back()));

			f.eval_stack.pop_back();

			c.vars[c.o_id].set_ptr(new Value(Handle<NativeFunction>(new LambdaReturnNativeFn(f))));

			f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, c));

		}

		if (uncallable) {
			f.rt->running->back()->eval_stack.emplace_back(v);
		} else {
			// trigger TypeError
		}

	}

}