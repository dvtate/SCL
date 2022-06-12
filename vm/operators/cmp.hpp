//
// Created by tate on 07-06-20.
//

#ifndef SCL_CMP_HPP
#define SCL_CMP_HPP

#include "operators.hpp"

// comparison functions

namespace VM_ops {

	/// check equality
	extern VMOperator dobule_equals;

	/// check identity
	extern VMOperator triple_equals;

	extern VMOperator lt;
	extern VMOperator gt;
	extern VMOperator le;
	extern VMOperator ge;

	// opposite of cmp identity/equality operators
	extern VMOperator not_double_equals;
	extern VMOperator not_triple_equals;
}


#endif //SCL_CMP_HPP
