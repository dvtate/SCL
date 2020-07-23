//
// Created by tate on 17-05-20.
//

#ifndef DLANG_HANDLE_HPP
#define DLANG_HANDLE_HPP

/* TODO remove this
 * This class is pretty fucking useless...
 * I would have just used normal pointers...
 * But that wouldn't be as clear what's managed by GC
 *
 */

#include "gc.hpp"

template <class T>
class Handle {
public:
	T* ptr;
	explicit Handle(T* p = nullptr): ptr(p) {}

	// Note: expects ptr to be GC-allocated pointer
	void set_ptr(T* ptr) {
		this->ptr = ptr;
	}
};

#endif //DLANG_HANDLE_HPP
