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
 * tfw using functions instead of objects
 */

/// Resumes execution of frame after async result received
class AsyncResultMsg : public virtual RTMessage {
public:
	Value ret;
	std::shared_ptr<SyncCallStack> stack_target;

	AsyncResultMsg(std::shared_ptr<Value> ret, std::shared_ptr<SyncCallStack> stack_target):
		ret(*ret), stack_target(std::move(stack_target)) {}

	void action(Runtime& rt) override {
		stack_target->back()->eval_stack.emplace_back(ret);

		// run it again bc should have been stopped when the Future was invoked
		rt.set_active(this->stack_target);
	}
	void mark() override {
		GC::mark(ret);
	}
};

/// typeof o within async call
class AsyncReturnNativeFn : public virtual NativeFunction {
public:
	// store return value if frame_target not set
	std::shared_ptr<Value> ret{nullptr};

	// this gets set when user invokes the future functor
	std::shared_ptr<SyncCallStack> stack_target{nullptr}; //

	// TODO mutex

	AsyncReturnNativeFn() {}

	void operator()(Frame& f) override {
		this->ret = std::make_shared<Value>(f.eval_stack.back());
		f.rt->kill_running();
		if (this->stack_target == nullptr)
			return;
		(*this->stack_target)[0]->rt->recv_msg(
				new AsyncResultMsg(this->ret, this->stack_target));
	}

	void mark() override {
		if (ret)
			GC::mark(*ret);
	}
};

/// typeof async(fn)(args)
class AsyncFutureNativeFn : public virtual NativeFunction {
public:
	AsyncReturnNativeFn* ofn;

	explicit AsyncFutureNativeFn(AsyncReturnNativeFn* ofn): ofn(ofn) {}

	void operator()(Frame& f) override {
		this->ofn->stack_target = f.rt->running;
		if (this->ofn->ret != nullptr) {
			f.eval_stack.emplace_back(*this->ofn->ret);
		} else {
			f.rt->freeze_running();
		}
	}

	void mark() override {
		ofn->mark();
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
			auto& c = *std::get<ValueTypes::lam_ref>(v.v);

			// get input (pass by reference vs value
			c.vars[c.i_id] =  f.eval_stack.back().type() == Value::VType::REF
				? std::get<Value::ref_t>(f.eval_stack.back().v)
				: ::new(GC::alloc<Value>()) Value(f.eval_stack.back());
			f.eval_stack.pop_back();

			// make output fn
			auto* ofn = ::new(GC::alloc<AsyncReturnNativeFn>()) AsyncReturnNativeFn();
			auto* future = ::new(GC::alloc<AsyncFutureNativeFn>()) AsyncFutureNativeFn(ofn);
			c.vars[c.o_id] = ::new(GC::alloc<Value>()) Value((NativeFunction*) ofn);

			// return future functor
			f.rt->running->back()->eval_stack.emplace_back((NativeFunction*)future);

			// context switch to other frame
			auto rcs = f.rt->running;
			f.rt->spawn_thread();
			f.rt->running->emplace_back(std::make_shared<Frame>(f.rt, c));
		} else {
			std::cerr <<"async only accepts closures for now :/\n";
			// todo: typerror
			// not a closure
			// just run it as tho it's sync lol
			vm_util::invoke_value_sync(f, this->v, true);
		}
	}

	void mark() override {
		GC::mark(this->v);
	}
};


#endif //DLANG_ASYNC_HPP
