//
// Created by tate on 23-05-20.
//

#include <iostream>
#include <dlfcn.h>
#include "global_ids.hpp"

#include "value_types.hpp"
#include "value.hpp"
#include "vm.hpp"
#include "operators/internal_tools.hpp"
#include "async.hpp"
#include "error.hpp"


//TODO: split this into multiple files...


class UnfreezeCallStack : public RTMessage {
public:
	std::shared_ptr<SyncCallStack> cs;
	explicit UnfreezeCallStack(std::shared_ptr<SyncCallStack> cs):
			cs(std::move(cs)) {}

	void action(Runtime& rt) override {
		rt.active.emplace_back(cs);
	}

	void mark() override {}
};

// a -> a returns same as input
class PrintFn : public NativeFunction {
public:
	void operator()(Frame& f) override {
		std::cout <<f.eval_stack.back().to_string() <<std::endl;
	}
	void mark() override {}
};

// Empty -> Str
class InputFn : public NativeFunction {
public:
	void operator()(Frame& f) override {
		// ignore inp, should be empty
		f.eval_stack.pop_back();

		std::shared_ptr<SyncCallStack> cs_sp = f.rt->running;

		// freeze thread until input received (eloop can work on other stuff)
		f.rt->freeze_running();

		std::thread([](const std::shared_ptr<SyncCallStack>& cs){
			std::string inp;
			if (!std::getline(std::cin, inp))
				cs->throw_error(gen_error_object("EOF-Error", "input not received", *cs));

			// return value
			cs->stack.back()->eval_stack.emplace_back(Value(inp));
			// unfreeze origin thread
			cs->stack.back()->rt->recv_msg(new UnfreezeCallStack(cs));
			// die
		}, cs_sp).detach();
	}

	void mark() override {}
};

class IfFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value i = f.eval_stack.back();
		f.eval_stack.back() = Value();
		SCL_DEBUG_MSG("if(" << i.to_string() << ") called");
		// invalid arg
		if (i.type() != Value::VType::LIST)
			return;

		// get values
		auto& params = *std::get<Value::list_ref>(i.v);

		// pick value
		const bool cond = params[0].truthy();
		unsigned char ind = cond ? 1 : 2;
		if (ind >= params.size())
			return;

		// call/push corresponding value
		vm_util::invoke_value_sync(f, params[ind], true);
	}

	void mark() override {}
};

// Any -> Str
class StrFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value& v = f.eval_stack.back();

		// if object has a __str method, call that
		if (v.type() == ValueTypes::VType::OBJ) {
			auto& o = *std::get<ValueTypes::obj_t*>(v.v);
			if (o.find("__str") != o.end()) {
				vm_util::invoke_value_sync(f, o["__str"], true);
				return;
			}
		}

		// Otherwise convert it to a string manually
		f.eval_stack.back() = Value(v.to_string());
	}

	void mark() override {}
};

class ImportFn : public NativeFunction {
public:
	void operator()(Frame& f) override {
		const std::string path = std::get<Value::str_t>(f.eval_stack.back().v);

		// import file
		void* dl = dlopen(path.c_str(), RTLD_LAZY);
		if (!dl) {
			std::cerr <<"Fatal: import error:" <<dlerror() <<std::endl;
			exit(1);
		}

		// import action
		using imported_fn = void(*)(Frame*);
		auto import_action = (imported_fn) dlsym(dl, "export_action");
		if (!import_action) {
			std::cerr <<"Fatal: import error: dlerror: " <<dlerror() <<std::endl;
			exit(1);
		}

		// Call the imported fn and let it take it's course
		import_action(&f);
	}

	void mark() override {}
};

// Any -> Int | Float | Empty
class NumFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value& in = f.eval_stack.back();

		// already a Num => return is
		if (std::holds_alternative<Value::int_t>(in.v) ||
			std::holds_alternative<Value::float_t>(in.v))
			return;

		// parse strings
		if (std::holds_alternative<Value::str_t>(in.v)) {
			const auto& s = std::get<Value::str_t>(in.v);
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
			// invalid => empty
			f.eval_stack.back() = Value();
		}
	}

	void mark() override {}
};

// Debug variables
class VarsFn : public NativeFunction {
	void operator()(Frame& f) override {
		for (const auto& scope : f.rt->running->stack) {
			std::cout <<"Scope " <<scope <<std::endl;
			for (const auto& vp : scope->closure.vars)
				std::cout <<'\t' <<vp.first <<'\t' <<vp.second->to_string(true)
						  <<'\t' <<(void*) vp.second <<std::endl;

		}
	}

	void mark() override {}
};

// Create async wrapper for closure (see async.hpp)
class AsyncFn : public NativeFunction {
	void operator()(Frame& f) override {
		f.eval_stack.back() = Value((NativeFunction*)
				f.gc_make<AsyncWrapperNativeFn>(f.eval_stack.back()));
	}
	void mark() override {}
};

class SizeFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value& v = f.eval_stack.back();
		switch (v.type()) {
			case ValueTypes::VType::STR:
				v = Value((ValueTypes::int_t) std::get<ValueTypes::str_t>(v.v).size());
				break;
			case ValueTypes::VType::LIST:
				v = Value((ValueTypes::int_t) std::get<ValueTypes::list_ref>(v.v)->size());
				break;
			case ValueTypes::VType::OBJ:
				v = Value((ValueTypes::int_t) std::get<ValueTypes::obj_ref>(v.v)->size());
				break;
			default:
				v = Value();
				break;
		}
	}
	void mark() override {}
};

class CopyFn : public NativeFunction {
	// TODO move this to Value class
	static Value copy_value(const Value& v, Frame& f) {
		do_branch:
		switch (v.type()) {
			// non-reference types
			case ValueTypes::VType::FLOAT:
			case ValueTypes::VType::INT:
			case ValueTypes::VType::EMPTY:
			case ValueTypes::VType::STR:
				return v;

			case ValueTypes::VType::LIST: {
				const auto* l = std::get<ValueTypes::list_ref>(v.v);
				auto* ret = f.gc_make<ValueTypes::list_t>();
				for (auto& e : *l)
					ret->emplace_back(copy_value(e, f));
				return Value(ret);
			};
			case ValueTypes::VType::OBJ: {
				const auto* o = std::get<ValueTypes::obj_ref>(v.v);
				auto* ret = f.gc_make<ValueTypes::obj_t>();
				for (const auto& p : *o)
					(*ret)[p.first] = copy_value(p.second, f);
				return Value(ret);
			};

			// TODO closures, native functions:
			case ValueTypes::VType::LAM: case ValueTypes::VType::N_FN:
				return v;

			case ValueTypes::VType::REF:
				throw "????";
		}
		return Value();
	}

public:
	void operator()(Frame& f) override {
		auto ret = copy_value(f.eval_stack.back(), f);
		f.eval_stack.back() = ret;
	}
	void mark() override {}
};

// Throw an error
class ThrowFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value e = f.eval_stack.back();
		f.eval_stack.pop_back();
		f.rt->running->throw_error(e);
	}
	void mark() override {}
};

// Error constructor
class ErrorFn : public NativeFunction {
	void operator()(Frame& f) override {
		Value& i = f.eval_stack.back();
		switch (i.type()) {
			// Extract arguments from lists and objects
			case ValueTypes::VType::LIST: {
				ValueTypes::list_t& l = *std::get<ValueTypes::list_t*>(i.v);
				std::string name = l[0].to_string();
				std::string message = l[1].to_string();
				f.eval_stack.back() = gen_error_object(name, message, f);
			}
			case ValueTypes::VType::OBJ: {
				ValueTypes::obj_t &o = *std::get<ValueTypes::obj_t *>(i.v);
				std::string name = o["name"].to_string();
				std::string message = o["message"].to_string();
				f.eval_stack.back() = gen_error_object(name, message, f);
			}

			// No arg = no message
			case ValueTypes::VType::EMPTY:
				f.eval_stack.back() = gen_error_object("Error", "", f);
				break;

			// Default name, given message
			default:
				f.eval_stack.back() = gen_error_object("Error", i.to_string(), f);
				break;
		}
	}
	void mark() override {}
};

static Value global_ids[] {
	// 0 - empty
	Value(),
	// 1 - print
	Value((NativeFunction*)::new(GC::static_alloc<PrintFn>()) PrintFn()),
	// 2 - input
	Value((NativeFunction*)::new(GC::static_alloc<InputFn>()) InputFn()),
	// 3 - if
	Value((NativeFunction*)::new(GC::static_alloc<IfFn>()) IfFn()),
	// 4 - Str
	Value((NativeFunction*)::new(GC::static_alloc<StrFn>()) StrFn()),
	// 5 - Num
	Value((NativeFunction*)::new(GC::static_alloc<NumFn>()) NumFn()),
	// 6 - vars
	Value((NativeFunction*)::new(GC::static_alloc<VarsFn>()) VarsFn()),
	// 7 - async
	Value((NativeFunction*)::new(GC::static_alloc<AsyncFn>()) AsyncFn()),
	// 8 - import
	Value((NativeFunction*)::new(GC::static_alloc<ImportFn>()) ImportFn()),
	// 9 - size
	Value((NativeFunction*)::new(GC::static_alloc<SizeFn>()) SizeFn()),
	// 10 - copy
	Value((NativeFunction*)::new(GC::static_alloc<CopyFn>()) CopyFn()),
	// 11 - Error
	Value((NativeFunction*)::new(GC::static_alloc<ErrorFn>()) ErrorFn()),
	// 12 - throw
	Value((NativeFunction*)::new(GC::static_alloc<ThrowFn>()) ThrowFn()),

	// TODO iterators?
};

// We want to use the more performant, segregated GC spaces
static_assert(sizeof(NativeFunction) == sizeof(PrintFn), "PrintFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(InputFn), "InputFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(IfFn), "IfFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(StrFn), "StrFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(NumFn), "NumFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(VarsFn), "VarsFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(AsyncFn), "AsyncFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(ImportFn), "ImportFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(SizeFn), "SizeFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(CopyFn), "CopyFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(ThrowFn), "ThrowFn wrong size");
static_assert(sizeof(NativeFunction) == sizeof(ThrowFn), "ThrowFn wrong size");
const Value& get_global_id(int64_t id) {
	return global_ids[id];
}