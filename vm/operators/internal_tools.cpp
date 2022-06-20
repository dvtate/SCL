//
// Created by tate on 31-05-20.
//

#include "../value.hpp"
#include "../lambda_return.hpp"
#include "../error.hpp"

#include "internal_tools.hpp"


namespace vm_util {

	void invoke_value_sync(Frame& f, Value& v, bool uncallable) {

		switch (v.type()) {
			case ValueTypes::VType::N_FN:
				// std::cout <<"invoke native..\n";
				(*std::get<ValueTypes::n_fn_ref>(v.v))(f);
				return;
			case ValueTypes::VType::LAM: {
				// Bind input
				Closure c = *std::get<ValueTypes::lam_ref>(v.v);
				if (f.eval_stack.back().type() != ValueTypes::VType::REF) {
					c.vars[c.i_id] = f.gc_make<Value>(f.eval_stack.back());
				} else {
					c.vars[c.i_id] = std::get<ValueTypes::ref_t>(f.eval_stack.back().v);
				}
				f.eval_stack.pop_back();

				// Bind output
				c.vars[c.o_id] = f.gc_make<Value>((NativeFunction*)
						f.gc_make<LambdaReturnNativeFn>(f));

				//
				f.rt->running->stack.emplace_back(new Frame(f.rt, c));
				return;
			}

			default:
				if (uncallable) {
					f.rt->running->stack.back()->eval_stack.back() = v;
				} else {
					f.rt->running->throw_error(gen_error_object("TypeError",
            			std::string("Cannot call value of type") + v.type_name() , f));
				}
		}
	}


	/// Numeric index, prolly list
	Value index_value(Frame& f, Value& v, ValueTypes::int_t index) {
		if (std::holds_alternative<ValueTypes::list_ref>(v.v)) {
			// Extract list
			const auto &list = *std::get<ValueTypes::list_ref>(v.v);

			// Python-style negative indices
			if (index < 0)
				index += list.size();

			// Out of range
			if (index >= list.size())
				return Value();

			// Return indexed value
			return list[index];
		} else if (std::holds_alternative<ValueTypes::str_t>(v.v)) {
			// Extract string
			const auto &str = std::get<ValueTypes::str_t>(v.v);

			// Python-style negative indices
			if (index < 0)
				index += str.size();

			// Out of range
			if (index >= str.size())
				return Value();

			// Return indexed value
			std::string ret = "a";
			ret[0] = str[index];
			return Value(ret);
		} else {
			f.rt->running->throw_error(gen_error_object(
					"TypeError",
					std::string("Numeric indices not available for type ") + v.type_name(),
					f));
			return Value();
		}
	}

	/// String index, prolly object
	Value index_value(Frame& f, Value& v, const ValueTypes::str_t& index) {
		if (std::holds_alternative<ValueTypes::obj_ref>(v.v)) {
			return (*std::get<ValueTypes::obj_ref>(v.v))[index];
		} else {
			// TODO attempt to access members of primitives
			return Value();
		}
	}

	Value index_value(Frame& f, Value& v, Value& index) {
//		std::cout <<"index_value(" << v.to_string() <<", " <<index.type_name() <<")\n";

		// Object with overloaded [] operator
		if (std::holds_alternative<ValueTypes::obj_ref>(v.v)) {
			auto& handler = (*std::get<ValueTypes::obj_ref>(v.v))["__operator[]"];
			const auto t = handler.type();
			if (t == ValueTypes::VType::LAM || t == ValueTypes::VType::N_FN) {
				f.eval_stack.emplace_back(index);
				invoke_value_sync(f, handler, false);
				Value ret = f.eval_stack.back();
				f.eval_stack.pop_back();
				return ret;
			}
		}

		// Handle indexing type
		switch (index.type()) {
			// List indexing
			case ValueTypes::VType::INT:
				return index_value(f, v, std::get<ValueTypes::int_t>(index.v));
			case ValueTypes::VType::FLOAT:
				return index_value(f, v, (ValueTypes::int_t) std::get<ValueTypes::float_t>(index.v));

			// Object indexing
			case ValueTypes::VType::STR:
				return index_value(f, v, std::get<ValueTypes::str_t>(index.v));

			// Type-Error
			default:
				f.rt->running->throw_error(gen_error_object(
						"TypeError",
						std::string("Invalid type used as index for [] operator - ") + index.type_name(),
						f));
				return Value();
		}
	}
}