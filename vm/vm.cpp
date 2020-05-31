//
// Created by tate on 17-05-20.
//

#include <iostream>

#include "vm.hpp"
#include "value.hpp"
#include "bc/exec_bc_instr.hpp"
#include "global_ids.hpp"


class ExitProgramReturn : public virtual NativeFunction {
public:

	void operator()(Frame& f) override {
		// TODO: convert stack.back() to int and return it
		// f.eval_stack.back();
		exit(-1);
	}
};


VM::VM(std::vector<Literal> lit_header, std::vector<std::string> argv)
{
	this->literals = std::move(lit_header);

	this->main_thread = std::make_shared<Runtime>(this);
	this->main_thread->running = std::make_shared<SyncCallStack>();
	this->main_thread->running->emplace_back(std::make_shared<Frame>(
			&*this->main_thread, Closure{}));
	Closure& main = this->main_thread->running->back()->closure;

	auto& entry = std::get<ClosureDef>(this->literals.back().v);
	main.body = &entry.body;

	Handle<NativeFunction> exit_fn(new ExitProgramReturn());
	main.vars[entry.o_id] = Handle(new Handle(new Value(exit_fn)));

	// TODO: capture command-line args
	std::string argv_list = std::string("cmd args coming soon");
	main.vars[entry.i_id] = Handle(new Handle(new Value(argv_list)));

	// declare locals
	main.declare_empty_locals(entry.decl_ids);

	// capture global variables
	for (int64_t id : entry.capture_ids) {
//		std::cout <<"capture id: " <<id <<std::endl;
		main.vars[id] = get_global_id(id);
	}
}

void VM::run() {
	main_thread->run();
}

// event loop
void Runtime::run() {

	while (true) {
		// run main stack until completion
		if (this->running->back()->tick()) {
			// if running is finished, do next active stack

			// nothing to do :/
			if (this->active.empty())
				break; // TODO: change when async implemented

			this->running = this->active.back();
			this->active.pop_back();
		}

		// handle actions messages
		if (this->_msg_queue.size()) {
			std::vector<RTMessage*> msgs = this->clear_msg_queue();
			for (RTMessage *m : msgs) {
				m->action(*this);
				delete(m);
			}
		}

	}

}