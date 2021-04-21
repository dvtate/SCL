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

void SyncCallStack::mark() {
	for (auto& f : this->stack)
		f->mark();
}

class ExitProgramReturn : public virtual NativeFunction {
public:

	void operator()(Frame& f) override {
		// TODO: convert stack.back() to int and return it
		// f.eval_stack.back();
		if (std::holds_alternative<Value::int_t>(f.eval_stack.back().v))
			exit(std::get<Value::int_t>(f.eval_stack.back().v));
		else
			exit(0);
	}
	void mark() override {}
};
static_assert(sizeof(ExitProgramReturn) == sizeof(NativeFunction), "Should be same size for convenience");


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
//		std::cout <<"cid" <<id <<std::endl;
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

	// declare locals
//	main.declare_empty_locals(entry.decl_ids);
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