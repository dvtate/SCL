//
// Created by tate on 23-05-20.
//

#include <iostream>
#include <dlfcn.h>

#include "global_ids.hpp"

#include "value.hpp"
#include "vm.hpp"
#include "operators/internal_tools.hpp"
#include "async.hpp"


//TODO: split this into multiple files...


class UnfreezeCallStack : public virtual RTMessage {
public:
	std::shared_ptr<SyncCallStack> cs;
	explicit UnfreezeCallStack(std::shared_ptr<SyncCallStack> cs):
			cs(std::move(cs)) {}

	void action(Runtime& rt) override {
		rt.active.emplace_back(cs);
	}
};

// a -> a returns same as input
class PrintFn : public virtual NativeFunction {
public:
	void operator()(Frame& f) override {
		Value& msg = f.eval_stack.back();
		std::cout <<msg.to_string() <<std::endl;
	}
};

// Empty -> Str
class InputFn : public virtual NativeFunction {
public:
	void operator()(Frame& f) override {
		// ignore inp, should be empty
		f.eval_stack.pop_back();

		std::shared_ptr<SyncCallStack> cs_sp = f.rt->running;

		// freeze thread until input received (eloop can work on other stuff)
		f.rt->freeze_running();

		std::thread([](const std::shared_ptr<SyncCallStack>& cs){
			std::string inp;
			if (!std::getline(std::cin, inp)) {
				// TODO: eof-error
			}

			// return value
			cs->back()->eval_stack.emplace_back(Value(inp));
			// unfreeze origin thread
			cs->back()->rt->recv_msg(new UnfreezeCallStack(cs));
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

		// get values
		auto& params = *std::get<Value::list_ref>(i.v).ptr;

		// pick velue
		const bool cond = params[0].truthy();
		unsigned char ind = cond ? 1 : 2;
		if (ind >= params.size())
			return;

		// call/push corresponding value
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

// Num -> Num
class DelayFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		// Get sleep duration (arg)
		using namespace std::chrono_literals;
		auto dur = 1ms;
		if (f.eval_stack.back().type() == Value::VType::FLOAT)
			dur *= std::get<Value::float_t>(f.eval_stack.back().v);
		else if (f.eval_stack.back().type() == Value::VType::INT)
			dur *= std::get<Value::int_t>(f.eval_stack.back().v);

		// freeze thread until delay expires (eloop can work on other stuff)
		std::shared_ptr<SyncCallStack> cs_sp = f.rt->running;
		f.rt->freeze_running();

		std::thread([dur, cs_sp](){
			std::this_thread::sleep_for(dur);

			// unfreeze origin thread
			cs_sp->back()->rt->recv_msg(new UnfreezeCallStack(cs_sp));
		}).detach();
	}
};

class ImportFn : public virtual NativeFunction {
public:
	void operator()(Frame&f) override {

		std::cout <<"impfn:\n";
		const std::string path = std::get<Value::str_t>(f.eval_stack.back().v);

		// import file
		void* dl = dlopen(path.c_str(), RTLD_LAZY);
		if (!dl) {
			std::cerr <<"Fatal: import error:" <<dlerror() <<std::endl;
			exit(1);
		}

		// import action
		using imported_fn = void(*)(Frame*);
		imported_fn import_action = (imported_fn) dlsym(dl, "export_action");
		if (!import_action) {
			std::cerr <<"Fatal: import error: dlerror: " <<dlerror() <<std::endl;
			exit(1);
		}
		std::cout <<"impfn:\n";

		// Call the imported fn and let it take it's course
		import_action(&f);
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
	void operator()(Frame& f) override {
		for (const auto& scope : *f.rt->running) {
			std::cout <<"Scope " <<scope <<std::endl;
			for (const auto& vp : scope->closure.vars)
				std::cout <<'\t' <<vp.first <<'\t' <<vp.second.get_ptr()->to_string()
						  <<'\t' <<vp.second.get_ptr() <<std::endl;

		}
	}
};

class AsyncFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		f.eval_stack.back() = Value(Handle<NativeFunction>(new AsyncWrapperNativeFn(f.eval_stack.back())));
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
	// 7 - async
	Handle(new Value(Handle<NativeFunction>(new AsyncFn()))),
	// 8 - import
	Handle(new Value(Handle<NativeFunction>(new ImportFn()))),

	// - range (need objects first...)
	// - copy
	// -
};

const Handle<Value>& get_global_id(int64_t id) {
	return global_ids[id];
}