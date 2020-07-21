//
// Created by tate on 17-05-20.
//

#include "handle.hpp"
#include "../value.hpp"

template<>
void Handle<NativeFunction>::mark() {
	GC::mark(this->ptr);
	this->ptr->mark();
}

template<>
void Handle<Value>::mark() {
	GC::mark(this->ptr);
	this->ptr->mark();
}

template<>
void Handle<Value::obj_t>::mark() {
	GC::mark(this->ptr);
	for (auto p : *this->ptr) {
		p.second.mark();
	}
}
