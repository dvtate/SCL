//
// Created by tate on 17/06/2021.
//

#ifndef SCL_STRINGIFY_HPP
#define SCL_STRINGIFY_HPP

#include <optional>
#include <unordered_set>
#include <sstream>

#include "../../vm/vm.hpp"
#include "../../vm/operators/internal_tools.hpp"
#include "../../vm/error.hpp"

#include "cyclic_refs_exception.hpp"

// Exported native funciton
static NativeFunction* stringify_nfn;

class JSONStringifyFn : public NativeFunction {

	// TODO handle cyclic references...
	static std::optional<std::string> act(
			const Value& v,
			Frame& f,
			std::unordered_set<const Value*>& s
	) {
		// Check cyclic refs
		if (s.contains(&v)) {
			throw CyclicRefsEx();
		} else {
			s.emplace(&v);
		}

		// Branch on ValueTypes
		switch (v.type()) {
			case ValueTypes::VType::STR:
				return std::string("\"") + std::get<ValueTypes::str_t>(v.v) + "\"";
			case ValueTypes::VType::EMPTY:
				return "null";

			case ValueTypes::VType::FLOAT: {
				std::stringstream ss;
				ss <<v.get<ValueTypes::float_t>();
				return ss.str();
				// return std::to_string(std::get<ValueTypes::float_t>(v.v));
			}
			case ValueTypes::VType::INT:
				return std::to_string(std::get<ValueTypes::int_t>(v.v));

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
				std::string ret = "[";
				try {
					ret += JSONStringifyFn::act(l[0], f, s).value_or("null");
				} catch (CyclicRefsEx& e) {
					e.push("-- in index zero of list");
					throw e;
				}

				// Add more comma separated elements
				for (int i = 1; i < l.size(); i++) {
					ret += ",";
					try {
						ret += JSONStringifyFn::act(l[i], f, s).value_or("null");
					} catch (CyclicRefsEx& e) {
						e.push("-- in index " + std::to_string(i) + " of list");
						throw e;
					}
				}

				// End list
				ret += "]";
				return ret;
			};

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
					auto val = JSONStringifyFn::act(it->second, f, s);
					if (val)
						ret +="\"" + it->first + "\":" + *val;

				} catch (CyclicRefsEx& e) {
					e.push("-- in field '" + it->first + "' of object");
					throw e;
				}

				// Put the rest of the kv pairs into the ret
				++it;
				for (; it != o.end(); ++it) {
					try {
						auto val = JSONStringifyFn::act(it->second, f, s);
						if (val)
							ret += ",\"" + it->first + "\":" + *val;
					} catch (CyclicRefsEx& e) {
						e.push("-- in field '" + it->first + "' of object");
						throw e;
					}
				}

				// Return completed object
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
	void operator()(Frame& f) override {
		auto &arg = f.eval_stack.back();
		try {
			std::unordered_set<const Value*> s;
			auto ret = JSONStringifyFn::act(arg, f, s);
			f.eval_stack.back() = !ret ? Value() : Value(*ret);
		} catch (CyclicRefsEx& e) {
			f.rt->running->throw_error(gen_error_object("TypeError", e.trace, f));
		}
	}

	void mark() override {}
};



#endif //SCL_STRINGIFY_HPP
