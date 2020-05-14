//
// Created by tate on 30-04-20.
//

#ifndef DLANG_REFERENCE_HPP
#define DLANG_REFERENCE_HPP

#include <vector>

class Frame;


/* Garbage-collected object
 *
 *
 */
template <class T>
class Handle {
public:
	T* ptr;
	Handle(const T v) {
		ptr = new T(v);
	}

	T& operator*();



};

#endif //DLANG_REFERENCE_HPP
