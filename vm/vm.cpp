//
// Created by tate on 17-05-20.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

#include "vm.hpp"
#include "value.hpp"
#include "bc/exec_bc_instr.hpp"
#include "global_ids.hpp"
#include "operators/internal_tools.hpp"


inline void fatal_exception(Frame& f) {
	std::cout <<"Fatal Exception:\n";
	(*std::get<ValueTypes::n_fn_t>(get_global_id((int64_t) GlobalId::STR).v))(f);
	(*std::get<ValueTypes::n_fn_t>(get_global_id((int64_t) GlobalId::PRINT).v))(f);
	exit(1);
}


void SyncCallStack::mark() {
	for (auto& f : this->stack)
		f->mark();
}

void SyncCallStack::throw_error(Value thrown)  {
	for (int i = this->stack.size() - 1; i >= 0; i--) {
		auto* h = this->stack[i]->error_handler;
		if (h != nullptr) {
			// Pop the stack back one level behind that of the handler
			// Replace the bad function call(stack) with the error handler
			// Note that this destroys back until the i-1-th frame
			// Note that the position of that frame will be the call that caused the error

			std::shared_ptr<Frame> f;
			if (i != 0) {
				this->stack.resize(i);
				f = this->stack.back();
			} else {
				this->stack.resize(1);
				f = this->stack.back();
//				f->pos = f->closure.body->size();
			}

			f->eval_stack.emplace_back(thrown);
			vm_util::invoke_value_sync(*f, *h, true);
			return;
		}
	}

	// No handlers, die
	this->stack.back()->eval_stack.emplace_back(thrown);
	fatal_exception(*this->stack.back());
}

class ExitProgramReturn : public NativeFunction {
public:

	void operator()(Frame& f) override {
		if (std::holds_alternative<Value::int_t>(f.eval_stack.back().v))
			exit(std::get<Value::int_t>(f.eval_stack.back().v));
		else
			exit(0);
	}
	void mark() override {}
};
static_assert(sizeof(ExitProgramReturn) == sizeof(NativeFunction), "Should be same size for convenience");

//class FatalExceptionHandler : public NativeFunction {
//public:
//	void operator()(Frame& f) override {
//		fatal_exception(f, f.eval_stack.back());
//	}
//};

VM::VM(std::vector<Literal> lit_header, const std::vector<std::string>& argv, std::istream& bytecode_source):
	bytecode_source(bytecode_source)
{
	this->literals = std::move(lit_header);

	this->main_thread = std::make_shared<Runtime>(this);
	this->main_thread->running = std::make_shared<SyncCallStack>();
	this->main_thread->running->stack.emplace_back(std::make_shared<Frame>(
			&*this->main_thread, Closure{}));
	this->main_thread->undead.emplace_back(this->main_thread->running);
	Closure& main = this->main_thread->running->stack.back()->closure;

	auto& entry = std::get<ClosureDef>(this->literals.back().v);
	main.body = entry.body;
	main.i_id = entry.i_id();
	main.o_id = entry.o_id();

	// capture global variables
	for (const int64_t id : entry.capture_ids) {
		SCL_DEBUG_MSG("capture global id # " << id << std::endl);
		main.vars[id] = ::new(GC::alloc<Value>()) Value(get_global_id(id));
	}

	// Load argv
	ValueTypes::list_ref args = ::new(GC::alloc<ValueTypes::list_t>()) ValueTypes::list_t();
	for (const std::string& s : argv)
		args->emplace_back(Value(s));

	main.vars[main.i_id] = ::new(GC::alloc<Value>()) Value(args);
	main.vars[main.o_id] = ::new(GC::alloc<Value>()) Value(::new(GC::alloc<NativeFunction>()) ExitProgramReturn());

	// TODO: capture command-line args
	Value argv_list{std::string("cmd args coming soon")};
	main.vars[main.i_id] = ::new(GC::alloc<Value>()) Value(argv_list);
}

void VM::run() {
	main_thread->run();
}

// event loop
void Runtime::run() {

	while (!this->undead.empty()) {
		// handle actions messages
		if (!this->_msg_queue.empty()) {
			std::vector<RTMessage*> msgs = this->clear_msg_queue();
			for (RTMessage *m : msgs) {
				m->action(*this);
				delete(m);
			}
		}

		// Maybe we need to GC
		if ((GC::size() - GC::last_gc_size) > GC::THRESHOLD) {
//			std::cout <<"DOGC!\n";
			const int start = GC::size();
			this->vm->do_gc();
			GC::last_gc_size = GC::size();
//			std::cout <<"before: " <<start <<" after: " <<GC::last_gc_size <<std::endl;
			const auto diff = (int) start - (int) GC::last_gc_size;
//			if (diff)
//				std::cout <<"freed: " <<diff <<std::endl;
		}

		// make sure we have something to do
		if (this->running == nullptr) {
			if (this->active.empty()) {
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(1ms);

			} else {
				SCL_DEBUG_MSG("VM:RT:Pulled Stack from active\n");
				this->running = this->active.back();
				this->active.pop_back();
			}
		} else {
			for (int i = 100; i != 0 && this->running != nullptr; i--) {
				if (this->running->stack.back()->tick()) {
					SCL_DEBUG_MSG("VM:RT:Frame: ran out of instructions\n");
					// function ran out of instructions to run...
					// 	implicitly return value on top of stack
					std::shared_ptr<Frame>& f = running->stack.back();
					Value* ret_fn = f->closure.vars[f->closure.o_id];
					if (std::holds_alternative<Value::n_fn_t>(ret_fn->v)) {
						ValueTypes::n_fn_t receiver = std::get<Value::n_fn_t>(ret_fn->v);
						(*receiver)(*f);
					} else {
						freeze_running();
					}
					break;
				}
			}
		}
	}

}