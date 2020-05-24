//
// Created by tate on 17-05-20.
//

#include "vm.hpp"
#include "exec_bc_instr.hpp"

VM::VM(std::vector<Literal> lit_header, std::vector<std::string> argv):
	literals(std::move(lit_header))
{
	// TODO: capture command-line args
	auto arg = Value(Handle<Value>(new Value("cmd args coming soon")));

	ClosureDef& entry = std::get<ClosureDef>(this->literals.back().v);
	Closure main;
	main.body = &entry.body;
	main.


}

