//
// Created by tate on 17-05-20.
//

#ifndef DLANG_HANDLE_HPP
#define DLANG_HANDLE_HPP

/*
 * These are defined to help with the following
 * - GC tracing
 * - Size segregation? (potential optimization)
 * - Hiding the GC API's from the rest of the VM
 */

#include "gc.hpp"

template <class T>
class Handle {
public:
	T* ptr;

	explicit Handle(): ptr(nullptr) {}
	explicit Handle(T* p): ptr(p) {}
	Handle(const Handle& other) = default;
	~Handle() = default;

	T* get_ptr() const {
		return this->ptr;
	}

	// Note: expects ptr to be GC-allocated pointer
	void set_ptr(T* ptr) {
		this->ptr = ptr;
	}

	void mark() {
		GC::mark(this->ptr);
	}
};


#endif //DLANG_HANDLE_HPP
