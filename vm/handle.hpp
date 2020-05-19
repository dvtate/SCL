//
// Created by tate on 17-05-20.
//

#ifndef DLANG_HANDLE_HPP
#define DLANG_HANDLE_HPP

// TODO: link w/ gc... make this
template <class T>
class Handle {
public:
	T* ptr;

	explicit Handle(): ptr(nullptr) {};
	explicit Handle(T* p): ptr(p) {};
	~Handle() = default;

};

#endif //DLANG_HANDLE_HPP
