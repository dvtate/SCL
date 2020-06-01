//
// Created by tate on 23-05-20.
//

#include <iostream>

#include "global_ids.hpp"

#include "value.hpp"
#include "vm.hpp"

#include "operators/internal_tools.hpp"
//TODO: split this into multiple files...

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
		explicit UnfreezeCallStack(std::shared_ptr<SyncCallStack> cs):
			cs(std::move(cs)) {}

		void action(Runtime& rt) override {
			rt.active.emplace_back(cs);
		}
	};

	void operator()(Frame& f) override {
		// ignore inp, should be empty
		f.eval_stack.pop_back();

		std::shared_ptr<SyncCallStack> cs_sp = f.rt->running;

		// freeze thread until input received (eloop can work on other stuff)
		f.rt->freeze_running();

		std::thread([](const std::shared_ptr<SyncCallStack>& cs_sp){
			std::string inp;
			if (!std::getline(std::cin, inp)) {
				// TODO: eof-error
			}

			// return value
			cs_sp->back()->eval_stack.emplace_back(Value(inp));
			// unfreeze origin thread
			cs_sp->back()->rt->recv_msg(new UnfreezeCallStack(cs_sp));
			// die
		}, cs_sp).detach();

	}
};

class IfFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		const Value i = f.eval_stack.back();

		// invalid arg
		if (i.type() != Value::VType::LIST)
			return;

		const auto& params = std::get<Value::list_t>(i.v);

		const bool cond = params[0].truthy();
		unsigned char ind = cond ? 1 : 2;
		if (ind >= params.size())
			return;

		Value v = params[ind];
		if (v.type() == Value::VType::REF) {
			Value* p = std::get<Value::ref_t>(v.v).get_ptr()->get_ptr();
			if (p == nullptr)
				f.eval_stack.emplace_back(v);
			else
				v = *p;
		}

	}
};

class WhileFn : public virtual NativeFunction {
	// decrease f.pos
	// place call to arg 2 onto call stack
	// continue execution
	void operator()(Frame& f) override {
		const Value i = f.eval_stack.back();

		// TODO
		if (i.type() == Value::VType::LIST) {

		} else { // yikes...

		}
	}

};

static Handle<Handle<Value>> global_ids[] {
	// 0 - empty
	Handle(new Handle(new Value())),
	// 1 - print
	Handle(new Handle(new Value(Handle<NativeFunction>(new PrintFn())))),
	// 2 - input
	Handle(new Handle(new Value(Handle<NativeFunction>(new InputFn())))),
	// 3 - if
	Handle(new Handle(new Value(Handle<NativeFunction>(new IfFn())))),

	// 4 - while
	// 5 - str
	// 6 - range
	//
};



Handle<Handle<Value>> get_global_id(int64_t id) {
	return global_ids[id];
}