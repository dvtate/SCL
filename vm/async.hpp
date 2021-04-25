//
// Created by tate on 05-07-20.
//

#ifndef SCL_ASYNC_HPP
#define SCL_ASYNC_HPP

#include <iostream>
#include "../debug.hpp"
#include "vm.hpp"
#include "value.hpp"
#include "operators/internal_tools.hpp"
#include "error.hpp"

/*
 * Async is of type :: (a -> b) -> (a -> (Empty -> b))
 * tfw using functions instead of objects
 */

// TODO should probably split into .hpp and .cpp

/// Resumes execution of frame after async result received
class AsyncResultMsg : public RTMessage {
public:
	Value ret;
	std::shared_ptr<SyncCallStack> stack_target;
	bool is_error;

	AsyncResultMsg(std::shared_ptr<Value> ret, std::shared_ptr<SyncCallStack> stack_target, bool is_error):
		ret(*ret), stack_target(std::move(stack_target)), is_error(is_error) {}

	void action(Runtime& rt) override {
		if (this->is_error) {
			stack_target->throw_error(ret);
		} else {
			stack_target->stack.back()->eval_stack.emplace_back(ret);
		}

		// run it again bc should have been stopped when the Future was invoked
		rt.set_active(this->stack_target);
	}
	void mark() override {
		GC::mark(ret);
	}
};

/// typeof o within async call
class AsyncReturnNativeFn : public NativeFunction {
public:
	// store return value if frame_target not set
	std::shared_ptr<Value> ret{nullptr};

	// does ret store an error?
	bool is_error{false};

	// this gets set when user invokes the future functor
	std::shared_ptr<SyncCallStack> stack_target{nullptr}; //

	// TODO mutex

	AsyncReturnNativeFn() {}

	void operator()(Frame& f) override {
		this->ret = std::make_shared<Value>(f.eval_stack.back());
		this->kill_thread(f);
	}
	void kill_thread(Frame& f) {
		f.rt->kill_running();
		if (this->stack_target == nullptr)
			return;
		if (this->is_error)
			this->extend_error();
		this->stack_target->stack[0]->rt->recv_msg(
			new AsyncResultMsg(this->ret, this->stack_target, is_error));
	}
	void mark() override {
		if (ret)
			GC::mark(*ret);
	}

	void extend_error() {
		// Try to extend error callstack with current trace if it's native error
		if (std::holds_alternative<ValueTypes::obj_ref>(this->ret->v)) {
			auto& obj = *std::get<ValueTypes::obj_ref>(this->ret->v);
			if (obj.find("__str") != obj.end()) {
				auto& str_fn_v = obj["__str"];
				if (std::holds_alternative<ValueTypes::n_fn_t>(str_fn_v.v)) {
					auto* trace_str_fn = dynamic_cast<ErrorTraceStrFn*>(
							std::get<ValueTypes::n_fn_t>(str_fn_v.v));
					if (trace_str_fn)
						trace_str_fn->trace.extend(*this->stack_target);
				}
			}
		}
	}
};

/// typeof async(fn)(args)
class AsyncFutureNativeFn : public NativeFunction {
public:
	AsyncReturnNativeFn* ofn;

	explicit AsyncFutureNativeFn(AsyncReturnNativeFn* ofn): ofn(ofn) {}

	void operator()(Frame& f) override {
		this->ofn->stack_target = f.rt->running;
		if (this->ofn->ret != nullptr) {
			if (this->ofn->is_error) {
				this->ofn->extend_error();
				f.rt->running->throw_error(*this->ofn->ret);
			} else {
				f.eval_stack.emplace_back(*this->ofn->ret);
			}
		} else {
			f.rt->freeze_running();
		}
	}

	void mark() override {
		ofn->mark();
	}
};

// By default errors thrown in async tasks will
class AsyncDefaultCatchFn : public NativeFunction {
	AsyncReturnNativeFn* ret;
public:

	explicit AsyncDefaultCatchFn(AsyncReturnNativeFn* ret): ret(ret) {}

	void operator()(Frame& f) override {
		// Copy the error
		std::shared_ptr<Value> err = std::make_shared<Value>(f.eval_stack.back());
		// Print a message
		std::cout <<"WARNING: Uncaught Exception in async task:\n";

		// Convert the error to a string and print it
		(*std::get<ValueTypes::n_fn_t>(get_global_id((int64_t) GlobalId::STR).v))(f);
		(*std::get<ValueTypes::n_fn_t>(get_global_id((int64_t) GlobalId::PRINT).v))(f);

		// Put the error into the return closure and mark that it's an error
		// This way the future will throw
		this->ret->ret = err;
		this->ret->is_error = true;

		// Transfer control
		this->ret->kill_thread(f);
	}

	void mark() override {
		this->ret->mark();
	}
};

/// typeof async(fn)
class AsyncWrapperNativeFn : public NativeFunction {
public:
	Value v;
	explicit AsyncWrapperNativeFn(const Value& callable): v(callable) {}

	// Async await invoke
	void operator()(Frame& f) override {
		SCL_DEBUG_MSG("invoked async wrapper");

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
			f.rt->running->stack.back()->eval_stack.emplace_back((NativeFunction*)future);

			// context switch to other frame
			auto rcs = f.rt->running;
			f.rt->spawn_thread();
			f.rt->running->stack.emplace_back(std::make_shared<Frame>(f.rt, c));
			f.rt->running->stack.back()->error_handler = ::new(GC::alloc<Value>()) Value(
					(NativeFunction*)::new(GC::alloc<AsyncDefaultCatchFn>()) AsyncDefaultCatchFn(ofn));
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


#endif //SCL_ASYNC_HPP
