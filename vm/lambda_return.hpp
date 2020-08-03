//
// Created by tate on 23-05-20.
//

#ifndef DLANG_LAMBDA_RETURN_HPP
#define DLANG_LAMBDA_RETURN_HPP


#include <iostream>
#include "../debug.hpp"
#include "vm.hpp"
#include "value.hpp"

/*
 * 	- when o() called:
 		- o is an id that corresponds to a native functor that when run,
 			creates a RTMessage Object and sends it to Runtime.recv_msg()
 		- when RTMessage is processed in between instruction ticks
 			- if msg.targeted_stack (shared_ptr<SyncCallStack) isn't in Runtime.running/active
 				- push it onto back of active stack
			- if msg.targeted_closure (Closure*) isn't in msg.targeted_stack
				- throw error/crash
			- pop items off msg.targeted_stack until reaching targeted_closure
			- pop off targeted_closure from stack too
			- push msg.return_value onto Frame.eval_stack
			- process next RTMessage(s)
			- go back to executing instructions
*/

/*
 * There is a lot of room for optimization here
 * - Noteably, reducing number of values stored
 * - Removing runtime safety checks performed at compile time
 */

// executed by rt event loop
class LambdaReturnMsg : public RTMessage {
public:
	std::shared_ptr<Frame> frame_target;
	std::shared_ptr<SyncCallStack> stack_target;
	Value ret;
	LambdaReturnMsg():
		frame_target(nullptr), stack_target(nullptr), ret() {}

	LambdaReturnMsg(
			std::shared_ptr<Frame> frame_target,
			std::shared_ptr<SyncCallStack> stack_target,
			Value& return_value):
		frame_target(std::move(frame_target)), stack_target(std::move(stack_target)), ret(return_value)
		{}

	void action(Runtime& rt) override {
		// TODO: Once compiler is smart enough to detect BS, this can be optimized

		if (!this->stack_target || !this->frame_target) {
			std::cout <<"invalid lambda return msg call ";
		}

		//
		while (this->stack_target->stack.back() != this->frame_target && !this->stack_target->stack.empty())
			this->stack_target->stack.pop_back();

		if (this->stack_target->stack.empty()) {
			std::cout <<"o() called out of frame???\n";
			return;
		}

		this->stack_target->stack.back()->eval_stack.push_back(this->ret);
		// if stack target not in active stacks, put there
		rt.set_active(this->stack_target);

//		// find frame on stack
//		ssize_t i;
//		for (i = this->stack_target->size() - 1; i >= 0; i--)
//			if ((*this->stack_target)[i] == this->frame_target)
//				break;
//		i++; // don't pop frame_target...
//		if (i <= 0) {
//			// no longer on stack wtf??
//			std::cout << "o() called out of scope???";
//			// TODO: o() out of scope error
//
//
//		} else {
//
//			// pop stack until back to call site
//			this->stack_target->erase(this->stack_target->begin() + i, this->stack_target->end());
//
//			// push return value onto top of stack
//			this->stack_target->back()->eval_stack.push_back(this->ret);
//
//			// if stack target not in active stacks, put there
//			if (rt.running != this->stack_target &&
//					std::find(rt.active.begin(), rt.active.end(), this->stack_target) == rt.active.end()) {
//				DLANG_DEBUG_MSG("LAM_RET: queued stack..\n");
//				rt.active.emplace_back(this->stack_target);
//			}
//
//		}

	}

	void mark() override {
		GC::mark(this->ret);
	}
};

// user-callable
class LambdaReturnNativeFn : public NativeFunction {
	std::shared_ptr<Frame> frame_target;
	std::shared_ptr<SyncCallStack> stack_target;
	std::shared_ptr<Runtime> rt;
public:
	explicit LambdaReturnNativeFn(Frame& f) {
		this->rt = f.rt->vm->main_thread; // TODO: detect thread/convert rt to weak_ptr
		this->frame_target = f.rt->running->stack.back();
		this->stack_target = f.rt->running;
	}


	void operator()(Frame& f) override {
		DLANG_DEBUG_MSG("o() called\n");
		this->rt->recv_msg(new LambdaReturnMsg(
				this->frame_target,
				this->stack_target,
				f.eval_stack.back()));

		// prevent double-return
		if (this->rt->running == this->stack_target)
			this->rt->freeze_running();
	}

	void mark() override { }
};

#endif //DLANG_LAMBDA_RETURN_HPP
