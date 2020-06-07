//
// Created by tate on 31-05-20.
//

#include "internal_tools.hpp"

#include "../value.hpp"


namespace vm_util {

	void invoke_value_sync(Frame &f, Value v, bool uncallable = true) {
		auto vt = v.type();
		if (vt == Value::VType::REF) {
			Value *p = std::get<Value::ref_t>(v.v).get_ptr()->get_ptr();
			if (p == nullptr && uncallable)
				f.eval_stack.emplace_back(v);
			if (p == nullptr) return; // Error: cannot invoke null
			else
				v = *p;
			vt = v.type();
		}

		if (vt == Value::VType::N_FN) {
			// std::cout <<"invoke native..\n";
			(*std::get<Handle<NativeFunction>>(v.v).ptr)(f);
			return;
		}
		if (vt == Value::VType::LAM) {
			auto *c = std::get<Value::lam_t>(v.v).get_ptr();
			Value arg = f.eval_stack.back();
			f.eval_stack.pop_back();

			// pass arg by reference
			if (std::holds_alternative<Value::ref_t>(arg.v))
				c->vars[c->i_id] = arg;
			else  // wasnt given reference, copy value into one :///
				c->vars[c->i_id] = Value(Handle(new Handle(new Value(arg))));

			f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, *c));

		}


		if (uncallable) {
			f.rt->running->back()->eval_stack.emplace_back(v);
		} else {
			// TypeError
		}

	}

}