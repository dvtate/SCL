//
// Created by tate on 26/06/2021.
//

#ifndef SCL_PRETTY_HPP
#define SCL_PRETTY_HPP

#include <unordered_set>

#include "cyclic_refs_exception.hpp"
#include "stringify.hpp"

static NativeFunction* encoder_ctr_fn;

class JSONPrettyStrFn : public NativeFunction {
	/// String to use for indentation
	std::string indent;

	/// Generate indentation string
	[[nodiscard]] std::string indent_str(unsigned ind_level) const {
		std::string ret;
		ret.reserve(ind_level * ret.size());
		for (unsigned i = 0; i < ind_level; i++)
			ret += this->indent;
		return ret;
	}

	/// Pretty print string
	std::optional<std::string> act(
		const Value& v,
		Frame& f,
		const unsigned indent_level,
		std::unordered_set<const Value*>& s
	) {
		// Check Cyclic refs
		if (s.contains(&v)) {
			throw CyclicRefsEx();
		} else {
			s.emplace(&v);
		}

		// Branch on ValueTypes
		switch (v.type()) {
			case ValueTypes::VType::STR:
				return std::string("\"") + std::get<ValueTypes::str_t>(v.v) + "\"";

			// Close enough
			case ValueTypes::VType::EMPTY:
				return "null";

			case ValueTypes::VType::FLOAT: {
				std::stringstream ss;
				ss <<v.get<ValueTypes::float_t>();
				return ss.str();
				// return std::to_string(std::get<ValueTypes::float_t>(v.v));
			}

			case ValueTypes::VType::INT:
				return std::to_string(v.get<ValueTypes::int_t>());

			// Can't do functions
			case ValueTypes::VType::N_FN:
			case ValueTypes::VType::LAM:
				return std::nullopt;

			// Arrays
			case ValueTypes::VType::LIST: {
				// Extract list
				auto& l = *std::get<ValueTypes::list_ref>(v.v);

				// Empty list edge case
				if (l.empty())
					return "[]";

				// Start with first element which doesn't need comma
				std::string ret = "[\n";
				try {
					ret += this->indent_str(indent_level);
					ret += this->act(l[0], f, indent_level + 1, s).value_or("null");
				} catch (CyclicRefsEx& e) {
					e.push("-- in index zero of list");
					throw e;
				}

				// Add more comma separated elements
				for (size_t i = 1; i < l.size(); i++) {
					ret += ",\n";
					ret += this->indent_str(indent_level);
					try {
						ret += this->act(l[i], f, indent_level + 1, s).value_or("null");
					} catch (CyclicRefsEx& e) {
						e.push("-- in index " + std::to_string(i) + " of list");
						throw e;
					}
				}

				// End list
				ret += "\n" + this->indent_str(indent_level - 1);
				ret += "]";
				return ret;
			}

			// Objects
			case ValueTypes::VType::OBJ: {
				auto& o = *std::get<ValueTypes::obj_ref>(v.v);
				if (o.empty())
					return "{}";

				// Custom to_json method/value
				/* either need better infrastructure or non-native impl
				if (o.contains("__to_json")) {
					auto& val = o["__to_json"];
					if (val.type() == ValueTypes::VType::LAM || val.type() == ValueTypes::VType::LAM) {
						f.eval_stack.emplace_back(Value());
//						std::cout <<"__to_json: " <<val.to_string(true) <<std::endl;
						vm_util::invoke_value_sync(f, val, false);
						return std::nullopt;
					}
				}
				*/

				// Normal object stringify same algorithm as for list
				auto it = o.begin();
				std::string ret = "{";
				try {
					// Put first kv pair into ret
					auto val = this->act(it->second, f, indent_level + 1, s);
					if (val)
						ret += "\n" + this->indent_str(indent_level)
								+ "\"" + it->first + "\": " + *val;
				} catch (CyclicRefsEx& e) {
					e.push("-- in field '" + it->first + "' of object");
					throw e;
				}

				// Put the rest of the kv pairs into the ret
				++it;
				for (; it != o.end(); ++it) {
					try {
						auto val = this->act(it->second, f, indent_level + 1,s);
						if (val)
							ret += ",\n" + this->indent_str(indent_level)
									+ "\"" + it->first + "\": " + *val;
					} catch (CyclicRefsEx& e) {
						e.push("-- in field '" + it->first + "' of object");
						throw e;
					}
				}

				// Return completed object
				ret += "\n" + this->indent_str(indent_level - 1);
				return ret + "}";
			}

			case ValueTypes::VType::REF: {
				f.rt->running->throw_error(gen_error_object("Error", "", f));
				return std::nullopt;
			}

			default:
				return std::nullopt;
		}
	}

public:
	explicit JSONPrettyStrFn(std::string indent = "\t"): indent(std::move(indent)) {}

	void operator()(Frame& f) override {
		auto& arg = f.eval_stack.back();
		try {
			std::unordered_set<const Value*> s;
			auto ret = this->act(arg, f, 1, s);
			f.eval_stack.back() = !ret ? Value() : Value(*ret);
		} catch (CyclicRefsEx& e) {
			f.rt->running->throw_error(gen_error_object("TypeError", e.trace, f));
		}
	}

	void mark() override {}
};


class JSONEncoderCtrFn : public NativeFunction {
public:
	void operator()(Frame& f) override {
		/*
		 * input: {
		 *    indent?: Str | Int = '\t',
		 * }
		 */

		// Get arg object
		auto& args = f.eval_stack.back();

		// Handle arguments
		std::string indent;
		if (std::holds_alternative<ValueTypes::empty_t>(args.v)) {
			indent = "\t";
		} else if (std::holds_alternative<ValueTypes::obj_ref>(args.v)) {
			const auto& obj = args.get<ValueTypes::obj_ref>();
			if (obj->contains("indent")) {
				auto& ind_arg = (*obj)["indent"];
				if (std::holds_alternative<ValueTypes::str_t>(ind_arg.v)) {
					indent = ind_arg.get<ValueTypes::str_t>();
				} else if (std::holds_alternative<ValueTypes::empty_t>(ind_arg.v)) {
					indent = "\t";
				} else if (std::holds_alternative<ValueTypes::int_t>(ind_arg.v)) {
					const auto n = ind_arg.get<ValueTypes::int_t>();
					for (int i = 0; i < n; i++)
						indent += ' ';
				} else {
					f.rt->running->throw_error(gen_error_object(
						"TypeError",
						"expected a string in field \'indent\'",
						f));
					return;
				}
			} else {
				indent = "\t";
			}
		} else {
			f.rt->running->throw_error(gen_error_object(
				"TypeError",
				"expected an options object argument as",
				f));
			return;
		}

		// Create object for it
		auto&& enc_fn = f.gc_make<JSONPrettyStrFn>(indent);
		ValueTypes::obj_t&& obj = {
			{ "encode", Value((NativeFunction*) enc_fn) }
		};
		f.eval_stack.back() = Value(f.gc_make<ValueTypes::obj_t>(obj));
	}

	void mark() override {}
};

#endif //SCL_PRETTY_HPP
