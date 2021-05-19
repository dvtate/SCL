//
// Created by tate on 22-05-20.
//

#include "operators.hpp"
#include "math.hpp"
#include "cmp.hpp"
#include "logic.hpp"

std::vector<VMOperator*> builtin_operators {
	&VM_ops::plus,				// 0  - +
	&VM_ops::minus,				// 1  - -
	&VM_ops::dobule_equals,		// 2  - ==
	&VM_ops::triple_equals,		// 3  - ===
	&VM_ops::lt,				// 4  - <
	&VM_ops::gt,				// 5  - >
	&VM_ops::le,				// 6  - <=
	&VM_ops::ge,				// 7  - >=
	&VM_ops::logical_not,		// 8  - !
};