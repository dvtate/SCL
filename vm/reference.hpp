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
	virtual Handle(Frame&, const T);

	T& operator*();

	virtual std::vector<void*> trace() {}

};

// Doesn't get free'd
// useful for
template <class T>
class StaticReference {
	StaticReference(const T);
};
#endif //DLANG_REFERENCE_HPP
