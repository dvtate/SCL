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
	explicit Handle(T* p = nullptr): ptr(p) {}
	Handle(const Handle& other) = default;
	~Handle() = default;

	// Note: expects ptr to be GC-allocated pointer
	void set_ptr(T* ptr) {
		this->ptr = ptr;
	}

	void mark() {
		GC::mark(this->ptr);
	}
};

class Value;
class NativeFunction;
template <> class Handle<NativeFunction>;
template <> class Handle<Value>;


#endif //DLANG_HANDLE_HPP
