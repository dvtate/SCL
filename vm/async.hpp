//
// Created by tate on 05-07-20.
//

#ifndef DLANG_ASYNC_HPP
#define DLANG_ASYNC_HPP

#include <iostream>
#include "../debug.hpp"
#include "vm.hpp"
#include "value.hpp"
#include "operators/internal_tools.hpp"

/*
 * Async is of type :: (a -> b) -> (a -> (Empty -> b))
 */

/// Resumes execution of frame after async result received
class AsyncResultMsg : public virtual RTMessage {
public:
	std::shared_ptr<Value> ret;
	std::shared_ptr<SyncCallStack> stack_target;

	AsyncResultMsg(std::shared_ptr<Value> ret, std::shared_ptr<SyncCallStack> stack_target):
		ret(std::move(ret)), stack_target(std::move(stack_target)) {}

	void action(Runtime& rt) override {
		stack_target->back()->eval_stack.emplace_back(*ret);

		// run it again bc should have been stopped when the Future was invoked
		rt.set_active(this->stack_target);
	}
};


class AsyncFutureNativeFn;

/// typeof o within async call
class AsyncReturnNativeFn : public virtual NativeFunction {
public:
	// store return value if frame_target not set
	std::shared_ptr<Value> ret{nullptr};

	// this gets set when user invokes the future functor
	std::shared_ptr<SyncCallStack> stack_target{nullptr}; //

	AsyncReturnNativeFn() {}

	void operator()(Frame& f) override {
		this->ret = std::make_shared<Value>(f.eval_stack.back());
		f.rt->kill_running();
		if (this->stack_target == nullptr) {
			return;
		}
		(*this->stack_target)[0]->rt->recv_msg(
				new AsyncResultMsg(this->ret, this->stack_target));
	}

	friend class AsyncFutureNativeFn;
};

/// typeof async(fn)(args)
class AsyncFutureNativeFn : public virtual NativeFunction {
public:
	ManagedHandle<AsyncReturnNativeFn> ofn;

	explicit AsyncFutureNativeFn(const ManagedHandle<AsyncReturnNativeFn>& ofn):
			ofn(ofn) {}

	~AsyncFutureNativeFn() {
		// This pointer is no longer managed by us it's now the GC's problem
		ofn.enable_gc();
	}

	void operator()(Frame& f) override {
		ofn.ptr->stack_target = f.rt->running;
		if (ofn.ptr->ret != nullptr) {
			f.eval_stack.emplace_back(*ofn.ptr->ret);
		} else {
			f.rt->freeze_running();
		}
	}
};

/// typeof async(fn)
class AsyncWrapperNativeFn : public virtual NativeFunction {
public:
	Value v;
	explicit AsyncWrapperNativeFn(const Value& callable): v(callable) {}

	// Async await invoke
	void operator()(Frame& f) override {
		DLANG_DEBUG_MSG("invoked async wrapper");

		if (this->v.type() == Value::VType::LAM) {
			// Make async lambda call

			// Get lambda
			auto& c = *std::get<Value::lam_t>(v.v).get_ptr();

			// get input
			// Pass by reference
			if (f.eval_stack.back().type() == Value::VType::REF)
				c.vars[c.i_id].set_ptr(std::get<Value::ref_t>(f.eval_stack.back().v).get_ptr());
			else
				c.vars[c.i_id].set_ptr(new Value(f.eval_stack.back()));
			f.eval_stack.pop_back();

			// make output fn
			auto* ofn = new AsyncReturnNativeFn();
			auto* future = new AsyncFutureNativeFn(ManagedHandle<AsyncReturnNativeFn>(
					ManagedHandle<AsyncReturnNativeFn>(ofn)));
			c.vars[c.o_id].set_ptr(new Value(Handle<NativeFunction>(ofn)));

			// return future functor
			f.rt->running->back()->eval_stack.emplace_back(Value(Handle<NativeFunction>(future)));

			// context switch to other frame
			auto rcs = f.rt->running;
			f.rt->freeze_running();
			f.rt->active.emplace_back(rcs);
			f.rt->running = std::make_shared<SyncCallStack>();
			f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, c));
		} else {
			// todo: typerror
			// not a closure
			// just run it as tho it's sync lol
			vm_util::invoke_value_sync(f, this->v, true);
		}

		// set o_id = AsyncReturnNativeFn
		// push running onto active stack
		// replace running with new SyncCallStack for call
		std::shared_ptr<SyncCallStack> cs = f.rt->running;
	}
};


#endif //DLANG_ASYNC_HPP