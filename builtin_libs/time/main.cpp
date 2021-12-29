//
// Created by tate on 03/07/2021.
//
#include <bitset>
#include "../../vm/operators/internal_tools.hpp"

#include "delay.hpp"

extern "C" void export_action(Frame* f) {
	auto* delay_fn = ::new(GC::static_alloc<ValueTypes::obj_t>()) DelayFn();

	f->eval_stack.back() = Value(
		::new(GC::static_alloc<ValueTypes::obj_t>())
			ValueTypes::obj_t(	{
				{ "delay", Value((NativeFunction*) delay_fn) }
			})
	);
}