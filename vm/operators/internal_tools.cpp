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
			Value::ref_t &receiver = std::get<Value::ref_t>(v.v);
			Value *p = receiver->ptr;
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
			Handle<NativeFunction> &receiver = std::get<Handle<NativeFunction>>(v.v);
			(*receiver->ptr)(f);
			return;
		}

		if (vt == Value::VType::LAM) {
			Value::lam_t &receiver = std::get<Value::lam_t>(v.v);
			auto c = *receiver->ptr;

			// Pass by reference
			if (f.eval_stack.back().type() == Value::VType::REF) {
				Value::ref_t &receiver1 = std::get<Value::ref_t>(f.eval_stack.back().v);
				c.vars[c.i_id].set_ptr(receiver1->ptr);
			}
			else
				c.vars[c.i_id].set_ptr(new Value(f.eval_stack.back()));

			f.eval_stack.pop_back();

			c.vars[c.o_id].set_ptr(new Value(Handle<NativeFunction>(new LambdaReturnNativeFn(f))));

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