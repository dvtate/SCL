//
// Created by tate on 17-05-20.
//

#include <iostream>

#include "vm.hpp"
#include "value.hpp"
#include "exec_bc_instr.hpp"
#include "global_ids.hpp"


class ExitProgramReturn : public NativeFunction {
public:

	void operator()(Frame& f) {
		// f.eval_stack.back();
		exit(-1);
	}
};


VM::VM(std::vector<Literal> lit_header, std::vector<std::string> argv)
{
	this->literals = std::move(lit_header);
	// TODO: capture command-line args
	std::string replace_with_argv = std::string("cmd args coming soon");
	auto arg = Handle<Value>(new Value(replace_with_argv));

	this->main_thread = std::make_shared<Runtime>(this);
	this->main_thread->running = std::make_shared<SyncCallStack>();
	this->main_thread->running->emplace_back(std::make_shared<Frame>(
			&*this->main_thread, Closure{}));
	Closure& main = this->main_thread->running->back()->closure;

	auto& entry = std::get<ClosureDef>(this->literals.back().v);
	main.body = &entry.body;
	main.vars[entry.i_id] = arg;
	Handle<NativeFunction> exit_fn(new ExitProgramReturn());
	main.vars[entry.o_id] = Handle<Value>(new Value(exit_fn));
	main.vars[entry.i_id] = arg;

	// capture global varaibles
	for (int64_t id : entry.capture_ids) {
		std::cout <<"capture id: " <<id <<std::endl;
		main.vars[id] = get_global_id(id);
	}
}

void VM::run() {
	main_thread->run();
}

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
			for (RTMessage *m : msgs)
				m->action(*this);

		}

	}

}