//
// Created by tate on 23-05-20.
//

#include <iostream>
#include <dlfcn.h>
#include <cassert>
#include "global_ids.hpp"

#include "value_types.hpp"
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

	void mark() override {}
};

// a -> a returns same as input
class PrintFn : public virtual NativeFunction {
public:
	void operator()(Frame& f) override {
		std::cout <<f.eval_stack.back().to_string() <<std::endl;
	}
	void mark() override {}
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

	void mark() override {}
};

class IfFn : public virtual NativeFunction {
	void operator()(Frame& f) override {

		Value& i = f.eval_stack.back();
		DLANG_DEBUG_MSG("if(" << i.to_string() <<") called");
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
class StrFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		f.eval_stack.back() = Value(f.eval_stack.back().to_string());
	}

	void mark() override {}
};

class ImportFn : public virtual NativeFunction {
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
		imported_fn import_action = (imported_fn) dlsym(dl, "export_action");
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

	void mark() override {}
};

// Debug variables
class VarsFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		for (const auto& scope : *f.rt->running) {
			std::cout <<"Scope " <<scope <<std::endl;
			for (const auto& vp : scope->closure.vars)
				std::cout <<'\t' <<vp.first <<'\t' <<vp.second->to_string()
						  <<'\t' <<(void*) vp.second <<std::endl;

		}
	}

	void mark() override {}
};

// Create async wrapper for closure (see async.hpp)
class AsyncFn : public virtual NativeFunction {
	void operator()(Frame& f) override {
		f.eval_stack.back() = Value((NativeFunction*)
				::new(GC::alloc<AsyncWrapperNativeFn>())
					AsyncWrapperNativeFn(f.eval_stack.back()));
	}
	void mark() override {}
};

class SizeFn : public virtual NativeFunction {
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

class CopyFn : public virtual NativeFunction {
	// TODO move this to Value class
	Value copy_value(const Value& v) {
		switch (v.type()) {
			// non-reference types
			case ValueTypes::VType::FLOAT:
			case ValueTypes::VType::INT:
			case ValueTypes::VType::EMPTY:
			case ValueTypes::VType::STR:
				return v;

			case ValueTypes::VType::LIST: {
				const auto* l = std::get<ValueTypes::list_ref>(v.v);
				ValueTypes::list_ref ret = ::new(GC::alloc<ValueTypes::list_t>()) ValueTypes::list_t(l->size());
				for (auto& e : *l)
					ret->emplace_back(copy_value(e));
				return Value(ret);
			};
			case ValueTypes::VType::OBJ: {
				const auto* o = std::get<ValueTypes::obj_ref>(v.v);
				ValueTypes::obj_ref ret = ::new(GC::alloc<ValueTypes::obj_t>()) ValueTypes::obj_t();
				for (const auto& p : *o)
					(*ret)[p.first] = copy_value(p.second);
				return Value(ret);
			};

			// TODO closures, native functions:
			case ValueTypes::VType::LAM: case ValueTypes::VType::N_FN:
				return v;
			default:
				throw "????";
		}
	}
	void operator()(Frame& f) override {
		f.eval_stack.back() = copy_value(f.eval_stack.back());
	}
	void mark() override {}
};


// TODO due to recycling behavior, these should not be GC'd
//  use normal values and copy them into GC'd Value*s as needed
static Value global_ids[] {
	// 0 - empty
	Value(::new(GC::alloc<Value>()) Value()),
	// 1 - print
	Value(::new(GC::alloc<PrintFn>()) PrintFn()),
	// 2 - input
	Value(::new(GC::alloc<InputFn>()) InputFn()),
	// 3 - if
	Value(::new(GC::alloc<IfFn>()) IfFn()),
	// 4 - Str
	Value(::new(GC::alloc<StrFn>()) StrFn()),
	// 5 - Num
	Value(::new(GC::alloc<NumFn>()) NumFn()),
	// 6 - vars
	Value(::new(GC::alloc<VarsFn>()) VarsFn()),
	// 7 - async
	Value(::new(GC::alloc<AsyncFn>()) AsyncFn()),
	// 8 - import
	Value(::new(GC::alloc<ImportFn>()) ImportFn()),
	// 9 - size
	Value(::new(GC::alloc<SizeFn>()) SizeFn()),
	// 10 - copy
	Value(::new(GC::alloc<CopyFn>()) CopyFn()),

	// - range (need objects first...)
	// - copy
	// - size
	//
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


const Value& get_global_id(int64_t id) {
	return global_ids[id];
}