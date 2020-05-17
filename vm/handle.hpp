//
// Created by tate on 17-05-20.
//

#ifndef DLANG_HANDLE_HPP
#define DLANG_HANDLE_HPP

template <class T>
class Handle {
public:
	T* _ptr;

	explicit Handle(): _ptr(nullptr) {};
	explicit Handle(T* p): _ptr(p) {};
};

#endif //DLANG_HANDLE_HPP
