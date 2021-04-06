//
// Created by tate on 31-05-20.
//

#ifndef SCL_INTERNAL_TOOLS_HPP
#define SCL_INTERNAL_TOOLS_HPP

#include "../value.hpp"
#include "../vm.hpp"


namespace vm_util {
	void invoke_value_sync(Frame& f, Value& v, bool uncallable);
}

#endif //SCL_INTERNAL_TOOLS_HPP
