//
// Created by tate on 19/05/2021.
//

#include "vm.hpp"
#include "value.hpp"
#include "error.hpp"

#include "primitive_methods.hpp"

/** TODO
 * Really there needs to be a bit more planning here
 * I'm trying to make an OOP style  solution in a lang that doesn't
 * have a clear, organized approach to OOP...
 */

// Remove leading and trailing whitespace
class StrTrimFn : public NativeFunction {
	std::string str;
public:
	explicit StrTrimFn(std::string&& str): str(str) {}

	void operator()(Frame& f) override {
		// start = index of first non-space character
		size_t start = 0;
		do {
			if (!isspace(this->str[start]))
				break;
		} while (++start < this->str.size());

		// end = index of last non-space character
		auto end = this->str.size() - 1;
		do {
			if (!isspace(this->str[start]))
				break;
		} while (--end > start);

		// substring onto stack
		f.eval_stack.back() = Value(str.substr(start, end - start));
	}
	void mark() override {}
};

class StrSplitFn : public NativeFunction {
	std::string str;
public:
	explicit StrSplitFn(std::string&& str): str(str) {}

	void operator()(Frame& f) override {
		// Get delimiter
		auto& arg = f.eval_stack.back();
		if (!std::holds_alternative<ValueTypes::str_t>(arg.v)) {
			f.rt->running->throw_error(gen_error_object(
					"ArgError",
					std::string("String.split() Expected a string delimiter, not ") + arg.type_name(),
					f));
			return;
		}
		auto& delimiter = std::get<ValueTypes::str_t>(arg.v);

		// Split the string
		size_t last = 0;
		size_t next;
		auto* ret = f.gc_make<ValueTypes::list_t>();
		while ((next = str.find(delimiter, last)) != std::string::npos) {
			ret->emplace_back(Value(str.substr(last, next-last)));
			last = next + 1;
		}
		ret->emplace_back(Value(str.substr(last)));

		// Return list of tokens
		f.eval_stack.back() = Value(ret);
	}
	void mark() override {}
};

Value get_primitive_member(Frame& f, Value& v, const std::string& key) {
	// TODO
	return Value();
}