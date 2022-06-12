//
// Created by tate on 31-05-20.
//

#ifndef SCL_INTERNAL_TOOLS_HPP
#define SCL_INTERNAL_TOOLS_HPP

#include "../value.hpp"
#include "../vm.hpp"

// TODO move these to be methods of the Frame class
namespace vm_util {
	/// Execute a value
	void invoke_value_sync(Frame& f, Value& v, bool uncallable);

	// Get the value stored in another value with given key
	Value index_value(Frame& f, Value& v, ValueTypes::int_t index);
	Value index_value(Frame& f, Value& v, const ValueTypes::str_t& index);
	Value index_value(Frame& f, Value& v, Value& index);
}

#endif //SCL_INTERNAL_TOOLS_HPP
