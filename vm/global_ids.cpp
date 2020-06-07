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
	static inline void printValue(Value& v) {
		if (std::holds_alternative<Value::ref_t>(v.v)) {
			Value* p = std::get<Value::ref_t>(v.v).get_ptr()->get_ptr();
			if (p == nullptr)
				std::cout <<"null" <<std::flush;
			else
				printValue(*p);
		} else if (std::holds_alternative<Value::str_t>(v.v)) {
			std::cout << std::get<Value::str_t>(v.v) <<std::flush;
		} else if (std::holds_alternative<Value::int_t>(v.v)) {
			std::cout <<std::get<Value::int_t>(v.v) <<std::flush;
		} else if (std::holds_alternative<Value::list_t>(v.v)) {
			auto l = std::get<Value::list_t>(v.v);

			std::cout <<"[ ";

			if (!l.empty())
				printValue(l[0]);

			if (l.size() > 1)
				for (std::size_t i = 1; i < l.size(); i++) {
					std::cout <<", ";
					printValue(l[i]);
				}

			std::cout <<" ]" <<std::flush;
		} else if (std::holds_alternative<Value::float_t>(v.v)) {
			std::cout <<std::get<Value::float_t>(v.v) <<std::flush;
		} else if (std::holds_alternative<Value::n_fn_t>(v.v)) {
			std::cout <<"<native procedure " << std::get<Value::n_fn_t>(v.v).get_ptr() << ">" << std::flush;
		} else if (std::holds_alternative<Value::lam_t>(v.v)) {
			std::cout <<"(: ... )" <<std::flush;
		} else if (std::holds_alternative<Value::empty_t>(v.v)) {
			std::cout <<"empty";
		} else {
			std::cout <<"Value of type: " <<v.type() <<std::flush;
		}
	}

	void operator()(Frame& f) override {
		Value msg = f.eval_stack.back();
		f.eval_stack.pop_back();
		PrintFn::printValue(msg);
		std::cout <<std::endl;
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
		f.eval_stack.pop_back();
		// invalid arg
		if (i.type() != Value::VType::LIST)
			return;

		const auto& params = std::get<Value::list_t>(i.v);

		const bool cond = params[0].truthy();
		unsigned char ind = cond ? 1 : 2;
		if (ind >= params.size())
			return;

		Value v = params[ind];

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

static Value global_ids[] {
	// 0 - empty
	Value(),
	// 1 - print
	Value(Handle<NativeFunction>(new PrintFn())),
	// 2 - input
	Value(Handle<NativeFunction>(new InputFn())),
	// 3 - if
	Value(Handle<NativeFunction>(new IfFn())),

	// 4 - while
	// 5 - str
	// 6 - range
	//
};

const Value& get_global_id(int64_t id) {
	return global_ids[id];
}