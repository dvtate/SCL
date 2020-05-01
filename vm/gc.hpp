//
// Created by tate on 30-04-20.
//

#ifndef DLANG_GC_HPP
#define DLANG_GC_HPP

#include <atomic>


#include <VM.h>

class VM;

struct gc_item {
	void* addr;

	enum TriColor {
		BLACK = 0, 	// gauranteed
		WHITE = 1,
		GREY = 2
	} mark;

	// reference count
	std::atomic<unsigned long> rc = 1;
};

void* gc_mallloc(VM& vm, std::size_t size);



#endif //DLANG_GC_HPP
