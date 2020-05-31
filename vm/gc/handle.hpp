//
// Created by tate on 17-05-20.
//

#ifndef DLANG_HANDLE_HPP
#define DLANG_HANDLE_HPP

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

	void set_value(const T& v) {
		*this->get_ptr() = v;
	}
};

template <class T>
class StaticHandle : public Handle<T> {

	// TODO: change constructor so that ptr isn't managed by gc
};


#endif //DLANG_HANDLE_HPP
