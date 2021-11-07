//
// Created by tate on 03/07/2021.
//

#ifndef SCL_DELAY_HPP
#define SCL_DELAY_HPP

#include "../../vm/vm.hpp"
#include "../../vm/error.hpp"

// Unfreezes thread, VM will decide when to resume running it
class UnfreezeThread : public virtual RTMessage {
public:
	std::shared_ptr<SyncCallStack> cs;
	explicit UnfreezeThread(std::shared_ptr<SyncCallStack> cs):
		cs(std::move(cs)) {}

	void action(Runtime& rt) override {
		rt.active.emplace_back(cs);
	}

	void mark() override {}
};

// This will be our export, it sleeps the current thread for given number of ms
//  but also lets other
class DelayFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
//		std::cout <<"delay\n";
		// Get sleep duration from user
		using namespace std::chrono_literals;
		auto duration = 1ms;
		if (f.eval_stack.back().type() == ValueTypes::VType::FLOAT) {
			duration *= std::get<ValueTypes::float_t>(f.eval_stack.back().v);
		} else if (f.eval_stack.back().type() == ValueTypes::VType::INT) {
			duration *= std::get<ValueTypes::int_t>(f.eval_stack.back().v);
		} else {
			f.rt->running->throw_error(gen_error_object("TypeError", "Expected a number", f));
			return;
		}
//		std::cout <<"dur: " <<duration.count() <<std::endl;

		// freeze thread
		std::shared_ptr<SyncCallStack> cs = f.rt->running;
		f.rt->freeze_running();

		// Spawn LWP and make it wait
		std::thread([duration, cs](){
			// kill time
//			std::cout <<"sleeping\n";
			std::this_thread::sleep_for(duration);
//			std::cout <<"waking\n";
			// unfreeze origin thread
			cs->stack.back()->rt->recv_msg(new UnfreezeThread(cs));
//			std::cout <<"done\n";
		}).detach();
	}

	void mark() override {}
};

#endif //SCL_DELAY_HPP
