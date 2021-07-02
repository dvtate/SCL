//
// Created by tate on 10/06/2021.
//

#include "stringify.hpp"
#include "parse.hpp"
#include "pretty.hpp"

extern "C" void export_action(Frame* f) {
	// TODO make these static vars
	if (!stringify_nfn)
		stringify_nfn = ::new(GC::static_alloc<NativeFunction>()) JSONStringifyFn();
	if (!parse_nfn)
		parse_nfn = ::new(GC::static_alloc<NativeFunction>()) JSONParseFn();
	if (!encoder_ctr_fn)
		encoder_ctr_fn = ::new(GC::static_alloc<NativeFunction>()) JSONEncoderCtrFn();

	f->eval_stack.back() = Value(::new(GC::static_alloc<ValueTypes::obj_t>()) ValueTypes::obj_t(
		{
			{ "encode", Value(stringify_nfn) },
			{ "decode", Value(parse_nfn) },
			{ "Encoder", Value(encoder_ctr_fn) }
		}
	));
}