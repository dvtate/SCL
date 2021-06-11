//
// Created by tate on 10/06/2021.
//

#include <optional>

#include "../../vm/vm.hpp"
// TODO this shouldn't be needed :(
#include "../../vm/operators/internal_tools.hpp"
#include "../../vm/error.hpp"

class JSONStringifyFn : public NativeFunction {
	// TODO handle cyclic references...
	static std::optional<std::string> act(const Value& v, Frame& f) {
		switch (v.type()) {
			case ValueTypes::VType::STR:
				return std::string("\"") + std::get<ValueTypes::str_t>(v.v) + "\"";
			case ValueTypes::VType::EMPTY:
				return "null";

			case ValueTypes::VType::FLOAT:
				return std::to_string(std::get<ValueTypes::float_t>(v.v));
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
				std::string ret = "[" + JSONStringifyFn::act(l[0], f).value_or("null");

				// Add more comma separated elements
				for (int i = 1; i < l.size(); i++) {
					ret += ",";
					ret += JSONStringifyFn::act(l[i], f).value_or("null");
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
				if (o.contains("__to_json")) {
					f.eval_stack.emplace_back(Value());
					vm_util::invoke_value_sync(f, o["__to_json"], true);
					Value ret = f.eval_stack.back();
					f.eval_stack.pop_back();
					return JSONStringifyFn::act(ret, f);
				}

				// Normal object stringify same algorithm as for list
				auto it = o.begin();
				std::string ret = "{";
				auto val = JSONStringifyFn::act(it->second, f);
				if (val)
					ret +="\"" + it->first + "\":" + *val;
				++it;

				for (; it != o.end(); ++it) {
					val = JSONStringifyFn::act(it->second, f);
					if (val)
						ret += ",\"" + it->first + "\":" + *val;
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
		auto ret = JSONStringifyFn::act(arg, f);
		if (!ret)
			f.eval_stack.back() = Value();
		else
			f.eval_stack.back() = Value(*ret);
	}

	void mark() override {}
};

extern "C" void export_action(Frame* f) {
	f->eval_stack.back() = Value(::new(GC::static_alloc<ValueTypes::obj_t>()) ValueTypes::obj_t(
		{
			{"to", Value(::new(GC::static_alloc<NativeFunction>()) JSONStringifyFn())},
		}));
}