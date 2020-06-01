//
// Created by tate on 17-05-20.
//

#include <iostream>
#include <thread>
#include <chrono>

#include "vm.hpp"
#include "value.hpp"
#include "bc/exec_bc_instr.hpp"
#include "global_ids.hpp"


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
};


VM::VM(std::vector<Literal> lit_header, std::vector<std::string> argv)
{
	this->literals = std::move(lit_header);

	this->main_thread = std::make_shared<Runtime>(this);
	this->main_thread->running = std::make_shared<SyncCallStack>();
	this->main_thread->running->emplace_back(std::make_shared<Frame>(
			&*this->main_thread, Closure{}));
	this->main_thread->undead.emplace_back(this->main_thread->running);
	Closure& main = this->main_thread->running->back()->closure;

	auto& entry = std::get<ClosureDef>(this->literals.back().v);
	main.body = &entry.body;
	main.i_id = entry.i_id;
	main.o_id = entry.o_id;

	// capture global variables
	for (int64_t id : entry.capture_ids) {
		if (id < 10)
			main.vars[id] = get_global_id(id);
	}

	Handle<NativeFunction> exit_fn(new ExitProgramReturn());
	main.vars[entry.o_id] = Handle(new Handle(new Value(exit_fn)));

	// TODO: capture command-line args
	Value argv_list{std::string("cmd args coming soon")};
	main.vars[entry.i_id] = Handle(new Handle(new Value(argv_list)));

	// declare locals
	main.declare_empty_locals(entry.decl_ids);


}

void VM::run() {
	main_thread->run();
}

// event loop
void Runtime::run() {

	while (!this->undead.empty()) {

		if (this->running == nullptr) {
			if (this->active.empty()) {
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(10ms);
			} else {
				std::cout <<"switch active\n";
				this->running = this->active.back();
				this->active.pop_back();
			}
			// run main stack until completion
		} else if (this->running->back()->tick()) {
			// if it got to end it must be awaiting return callback
			// so we freeze it to process other stacks
			this->freeze_running();
		}

		// handle actions messages
		if (!this->_msg_queue.empty()) {
			std::vector<RTMessage*> msgs = this->clear_msg_queue();
			for (RTMessage *m : msgs) {
				m->action(*this);
				delete(m);
			}
		}

	}

}