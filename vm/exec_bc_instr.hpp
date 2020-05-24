//
// Created by tate on 22-05-20.
//

#ifndef DLANG_EXEC_BC_INSTR_HPP
#define DLANG_EXEC_BC_INSTR_HPP

#include "bc.hpp"
class Frame;

void exec_bc_instr(Frame& f, BCInstr cmd);

#endif //DLANG_EXEC_BC_INSTR_HPP
