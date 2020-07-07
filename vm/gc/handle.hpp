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
// TODO: link w/ gc...


template <class T>
class Handle {
public:
	T* ptr;

	explicit Handle(): ptr(nullptr) {}
	explicit Handle(T* p): ptr(p) {}
	Handle(const Handle& other): ptr(other.ptr) {}
	~Handle() = default;

	T* get_ptr() const {
		// return (T*)((char*) this->ptr + 1);
		return this->ptr;
	}

	// Note: expects ptr to be GC-allocated pointer
	void set_ptr(T* ptr) {
		this->ptr = ptr;
	}
};

// this type stores data that may become untracable but is still being managed by some other data-structure
template <class T>
class ManagedHandle : public Handle<T> {
public:
	// TODO: change constructor so that ptr isn't managed by gc
	ManagedHandle() {
		this->ptr = nullptr;
	}
	explicit ManagedHandle(T* p) {
		this->ptr = p;
	}

	//
	void enable_gc() {
		// let gc manage this pointer
		// probably will need some GC api to be implemented
	}
};


#endif //DLANG_HANDLE_HPP
