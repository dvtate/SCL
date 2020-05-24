//
// Created by tate on 23-05-20.
//

#ifndef DLANG_LAMBDA_RETURN_HPP
#define DLANG_LAMBDA_RETURN_HPP


#include <iostream>
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
class LambdaReturnMsg : public virtual RTMessage {
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
		if (!this->stack_target || !this->frame_target) {
			std::cout <<"invalid lambda return msg call ";
		}

		// if stack target not in active stacks
		if (rt.running != this->stack_target &&
			std::find(rt.active.begin(), rt.active.end(), this->stack_target) == rt.active.end())
			rt.active.emplace_back(this->stack_target);

		// find frame on stack
		size_t i;
		for (i = this->stack_target->size(); i > 0; i--) {
			if ((*this->stack_target)[i] == this->frame_target)
				break;
		}
		if (i == 0) {
			// no longer on stack wtf??
			std::cout <<"Return called out of scope???";
		} else {
			// pop stack
			for (; i <= this->stack_target->size(); i++)
				this->stack_target->pop_back();

			// push return value onto top of stack
			this->stack_target->back()->eval_stack.emplace_back(this->ret);
		}

	}
};

class LambdaReturnNativeFn : public virtual NativeFunction {
	std::shared_ptr<Runtime> rt;
	LambdaReturnMsg msg;

	LambdaReturnNativeFn(): rt(nullptr), msg() {}
	explicit LambdaReturnNativeFn(Frame& f) {
		this->rt = f.rt->vm->main_thread; // TODO: detect thread/convert rt to weak_ptr
		this->msg = LambdaReturnMsg(f.rt->running->back(), f.rt->running, f.eval_stack.back());
	}

	void operator()(Frame& f) override {
		this->rt->recv_msg(new LambdaReturnMsg(this->msg));
	}
};

#endif //DLANG_LAMBDA_RETURN_HPP
