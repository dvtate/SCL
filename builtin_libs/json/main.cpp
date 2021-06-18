//
// Created by tate on 10/06/2021.
//

#include "stringify.hpp"
#include "parse.hpp"

extern "C" void export_action(Frame* f) {
	stringify_nfn = ::new(GC::static_alloc<NativeFunction>()) JSONStringifyFn();
	parse_nfn = ::new(GC::static_alloc<NativeFunction>()) JSONParseFn();
	f->eval_stack.back() = Value(::new(GC::static_alloc<ValueTypes::obj_t>()) ValueTypes::obj_t(
		{
			{ "dumps", Value(stringify_nfn) },
			{ "loads", Value(parse_nfn) },
		}
	));
}