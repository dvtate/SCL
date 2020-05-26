//
// Created by tate on 17-05-20.
//


#include "closure.hpp"
#include "value.hpp"

void Closure::declare_empty_locals(const std::vector<int64_t>& ids) {
	for (const int64_t id : ids)
		// NOTE: SIDE-EFFECT: if function gets called repeatedly vars will retain old values
		this->vars[id] = Handle( new Handle<Value>(new Value()));
}