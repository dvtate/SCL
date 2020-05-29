//
// Created by tate on 23-05-20.
//

#include <iostream>

#include "global_ids.hpp"

#include "value.hpp"
#include "vm.hpp"


class PrintFn : public virtual NativeFunction {
public:
	void operator()(Frame& f) override {
		Value msg = f.eval_stack.back();
		f.eval_stack.pop_back();
		if (std::holds_alternative<Value::str_t>(msg.v)) {
			std::cout << std::get<std::string>(msg.v) <<std::endl;
		}
	}
};

class InputFn : public virtual NativeFunction {
public:

	class UnfreezeCallStack : public virtual RTMessage {
	public:

		std::shared_ptr<SyncCallStack> cs;
		UnfreezeCallStack(std::shared_ptr<SyncCallStack> cs): cs(std::move(cs)) {}

		void action(Runtime& rt) override {
			rt.active.emplace_back(cs);
		}
	};


	void operator()(Frame& f) override {
		// no input needed, should be empty
		f.eval_stack.pop_back();

		std::shared_ptr<SyncCallStack> cs_sp = f.rt->running;
		f.rt->freeze_running();
		auto t = std::thread([](std::shared_ptr<SyncCallStack> cs_sp){
			std::string inp;
			if (!std::getline(std::cin, inp)) {
				// TODO: EOF error
			}
			cs_sp->back()->eval_stack.emplace_back(Value(inp));
			cs_sp->back()->rt->recv_msg(new UnfreezeCallStack(cs_sp));

		}, cs_sp);


	}
};


static Handle<Handle<Value>> global_ids[] {
	// 0 - empty
	Handle(new Handle(new Value())),
	// 1 - print
	Handle(new Handle(new Value(Handle<NativeFunction>(new PrintFn())))),
	// 2 - input
	// 3 - if
	// 4 - while
	// 5 - str
	// 6 - range
	//
};



Handle<Handle<Value>> get_global_id(int64_t id) {
	return global_ids[id];
}