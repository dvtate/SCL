//
// Created by tate on 19/05/2021.
//

#include "vm.hpp"
#include "value.hpp"
#include "error.hpp"
#include "gc/gc.hpp"

#include "primitive_methods.hpp"

/* TODO
 * Really there needs to be a bit more planning here
 * I'm trying to make an OOP-style solution in a lang that doesn't
 * have a clear, organized approach to OOP...
 */

// Remove leading and trailing whitespace
class StrTrimFn : public NativeClosure {
public:
	explicit StrTrimFn(std::string str) {
		this->data = (void*) new std::string(str);
	}
	virtual ~StrTrimFn() {
		delete (std::string*) this->data;
	}

	void operator()(Frame& f) override {
		// start = index of first non-space character
		size_t start = 0;
		std::string& str = *(std::string*)this->data;
		do {
			if (!isspace(str[start]))
				break;
		} while (++start < str.size());

		// end = index of last non-space character
		auto end = str.size() - 1;
		do {
			if (!isspace(str[end]))
				break;
		} while (--end > start);

		// substring onto stack
		f.eval_stack.back() = Value(str.substr(start, 1 + end - start));
	}
	void mark() override {}
};

class StrSplitFn : public NativeClosure {
public:
	explicit StrSplitFn(std::string str) {
		this->data = (void*) new std::string(str);
	}
	virtual ~StrSplitFn() {
		delete (std::string*) this->data;
	}

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
		std::string& str = *(std::string*)this->data;
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

/* TODO
Str: startsWith, endsWith, substr, find
List: foreach, map, reduce, ?
*/

Value get_primitive_member(Frame& f, Value& v, const std::string& key) {
	switch (v.type())
	{
	case ValueTypes::VType::STR:
		{			
			auto s = v.get<std::string>();
			if (key == "split")
				return Value(::new(f.rt->vm->gc.alloc<NativeClosure>()) StrSplitFn(s));
			else if (key == "trim")
				return Value(::new(f.rt->vm->gc.alloc<NativeClosure>()) StrTrimFn(s));
			else
				f.rt->running->throw_error(gen_error_object(
					"TypeError",
					std::string("cannot accesss property") + key + " of Str",
					f));
		}
		break;

	default:
		break;
	}
	return Value();
}