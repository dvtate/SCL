//
// Created by tate on 31-05-20.
//

#ifndef DLANG_INTERNAL_TOOLS_HPP
#define DLANG_INTERNAL_TOOLS_HPP

#include "../value.hpp"
#include "../vm.hpp"


namespace vm_util {
	const Value* deref(const Value&);
	void invoke_value_sync(Frame& f, bool uncallable=true);
}

#endif //DLANG_INTERNAL_TOOLS_HPP
