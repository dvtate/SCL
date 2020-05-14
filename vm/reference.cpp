//
// Created by tate on 30-04-20.
//

#include "reference.hpp"

Handle::Handle(const T v) {
	ptr = new T(v);
}