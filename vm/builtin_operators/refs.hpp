//
// Created by tate on 25-05-20.
//

#ifndef DLANG_REFS_HPP
#define DLANG_REFS_HPP

#include "../operators.hpp"

namespace VM_ops {
	// change value
	extern VMOperator single_equals;

	// change reference
	extern VMOperator colon_equals;

	// check equality
	extern VMOperator dobule_equals;

	// check identity
	extern VMOperator triple_equals;

}


#endif //DLANG_REFS_HPP
