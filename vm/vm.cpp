//
// Created by tate on 17-05-20.
//

#include "vm.hpp"
#include "value.hpp"
#include "exec_bc_instr.hpp"


class ExitProgramReturn : public NativeFunction {
public:

	void operator()(Frame& f) {
		// f.eval_stack.back();
		exit(-1);
	}
};


VM::VM(std::vector<Literal> lit_header, std::vector<std::string> argv):
	literals(std::move(lit_header))
{
	// TODO: capture command-line args
	std::string replace_with_argv = std::string("cmd args coming soon");
	auto arg = Handle<Value>(new Value(replace_with_argv));

	ClosureDef& entry = std::get<ClosureDef>(this->literals.back().v);
	Closure main;
	main.body = &entry.body;
	main.vars[entry.i_id] = arg;
	Handle<NativeFunction> exit_fn(new ExitProgramReturn());
	main.vars[entry.o_id] = Handle<Value>(new Value(exit_fn));
	main.vars[entry.i_id] = arg;


}

