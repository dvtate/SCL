//
// Created by tate on 23-05-20.
//

#include <iostream>

#include "global_ids.hpp"

#include "value.hpp"
#include "vm.hpp"
#include "operators/internal_tools.hpp"


//TODO: split this into multiple files...


// a -> a returns same as input
class PrintFn : public virtual NativeFunction {
public:
	static inline void printValue(Value& v) {
		if (std::holds_alternative<Value::ref_t>(v.v)) {
			Value* p = std::get<Value::ref_t>(v.v).get_ptr();
			if (p == nullptr)
				std::cout <<"null";
			else
				printValue(*p);
		} else if (std::holds_alternative<Value::str_t>(v.v)) {
			std::cout << std::get<Value::str_t>(v.v);
		} else if (std::holds_alternative<Value::int_t>(v.v)) {
			std::cout <<std::get<Value::int_t>(v.v);
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

			std::cout <<" ]";
		} else if (std::holds_alternative<Value::float_t>(v.v)) {
			std::cout <<std::get<Value::float_t>(v.v);
		} else if (std::holds_alternative<Value::n_fn_t>(v.v)) {
			std::cout <<"<native procedure " << std::get<Value::n_fn_t>(v.v).get_ptr() << ">";
		} else if (std::holds_alternative<Value::lam_t>(v.v)) {
			std::cout <<"(: ... )";
		} else if (std::holds_alternative<Value::empty_t>(v.v)) {
			std::cout <<"empty";
		} else {
			std::cout <<"Value of type: " <<v.type();
		}
	}

	void operator()(Frame& f) override {
		Value& msg = f.eval_stack.back();
//		PrintFn::printValue(msg);
		std::cout <<msg.to_string() <<std::endl;
	}
};

// Empty -> Str
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

		Value i = f.eval_stack.back();
		DLANG_DEBUG_MSG("if(" << i.to_string() <<") called");
		// invalid arg
		if (i.type() != Value::VType::LIST)
			return;

		auto& params = std::get<Value::list_t>(i.v);

		const bool cond = params[0].truthy();
		unsigned char ind = cond ? 1 : 2;
		if (ind >= params.size())
			return;

		vm_util::invoke_value_sync(f, params[ind], true);

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

// Any -> Str
class StrFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		f.eval_stack.back() = Value(f.eval_stack.back().to_string());
	}
};


// Any -> Int | Float | Empty
class NumFn : public virtual NativeFunction {

	void operator()(Frame& f) override {
		Value i = f.eval_stack.back();
		Value* in = i.deref();
		if (in == nullptr) {
			f.eval_stack.back() = Value();
			return;
		}
		if (std::holds_alternative<Value::int_t>(in->v) ||
			std::holds_alternative<Value::float_t>(in->v))
			// already a Num
			return;

		if (std::holds_alternative<Value::str_t>(in->v)) {
			const auto& s = std::get<Value::str_t>(in->v);
			try {
				f.eval_stack.back() = Value((Value::int_t) std::stoll(s));
			} catch (...) {
				try {
					f.eval_stack.back() = Value((Value::float_t) std::stod(s));
				} catch (...) {
					f.eval_stack.back() = Value();
				}
			}
		} else {
			f.eval_stack.back() = Value();
		}
	}
};

class VarsFn : public virtual NativeFunction {
	void operator()(Frame& f) {
		for (const auto& scope : *f.rt->running) {
			std::cout <<"Scope " <<scope <<std::endl;
			for (const auto& vp : scope->closure.vars)
				std::cout <<'\t' <<vp.first <<'\t' <<vp.second.get_ptr()->to_string()
						  <<'\t' <<vp.second.get_ptr() <<std::endl;

		}
	}
};



static Handle<Value> global_ids[] {
	// 0 - empty
	Handle(new Value()),
	// 1 - print
	Handle(new Value(Handle<NativeFunction>(new PrintFn()))),
	// 2 - input
	Handle(new Value(Handle<NativeFunction>(new InputFn()))),
	// 3 - if
	Handle(new Value(Handle<NativeFunction>(new IfFn()))),
	// 4 - Str
	Handle(new Value(Handle<NativeFunction>(new StrFn()))),
	// 5 - Num
	Handle(new Value(Handle<NativeFunction>(new NumFn()))),
	// 6 - vars
	Handle(new Value(Handle<NativeFunction>(new VarsFn()))),

	// - range (need objects first...)
	// - copy
	// -
};

const Handle<Value>& get_global_id(int64_t id) {
	return global_ids[id];
}