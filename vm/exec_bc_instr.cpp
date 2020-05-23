//
// Created by tate on 22-05-20.
//

#include "exec_bc_instr.hpp"
#include "vm.hpp"
#include "operators.hpp"

static inline void useId

void exec_bc_instr(Frame& f, BCInstr cmd) {
	switch (cmd.instr) {
		case BCInstr::OPCode::USE_ID:
			f.eval_stack.emplace_back(f.closure.vars[cmd.i]);
			return;
		case BCInstr::OPCode::BUILTIN_OP:


	}
}