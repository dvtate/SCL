//
// Created by tate on 23-05-20.
//

#include <iostream>

#include "global_ids.hpp"

#include "value.hpp"
#include "vm.hpp"


class PrintFn : public virtual NativeFunction {
public:
	void operator()(Frame& f) override {
		Value msg = f.eval_stack.back();
		f.eval_stack.pop_back();
		if (std::holds_alternative<Value::str_t>(msg.v)) {
			std::cout << std::get<std::string>(msg.v) <<std::endl;
		}
	}
};


class ToStringFn : public virtual NativeFunction {
public:

};

static Handle<Handle<Value>> global_ids[] {
	Handle( new Handle(new Value())), // empty
	Handle(new Handle(new Value(Handle<NativeFunction>(new PrintFn())))), // print

};


Handle<Handle<Value>> get_global_id(int64_t id) {
	return global_ids[id];
}