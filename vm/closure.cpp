//
// Created by tate on 17-05-20.
//


#include "closure.hpp"
#include "value.hpp"

void Closure::declare_empty_locals(const std::vector<int64_t>& ids) {
	// set all variables except the first two (i, o) reference empty
	for (std::size_t i = 2; i < ids.size(); i++) {
		// NOTE: SIDE-EFFECT: if closure gets called repeatedly vars will retain old values as this method isn't called again
		this->vars[ids[i]] = Value(Handle( new Handle<Value>(new Value())));
	}
}