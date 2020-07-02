//
// Created by tate on 31-05-20.
//

#ifndef DLANG_INTERNAL_TOOLS_HPP
#define DLANG_INTERNAL_TOOLS_HPP

#include "../value.hpp"
#include "../vm.hpp"


namespace vm_util {
	void invoke_value_sync(Frame& f, Value& v, bool uncallable);
}

#endif //DLANG_INTERNAL_TOOLS_HPP
