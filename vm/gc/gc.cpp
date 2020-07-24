//
// Created by tate on 30-04-20.
//

#include "../value.hpp"
#include "../vm.hpp"
#include "../closure.hpp"
#include "gc.hpp"

namespace GC {

// Declare tracking containers
#define DLANG__GC_HEAP(TYPE, NAME) \
	std::vector<TYPE*> heap_NAME; \
	std::vector<TYPE*> recycle_bin_NAME;

// Function template specializations for given type
#define DLANG__GC_SPEC(TYPE, NAME) \
	template<> \
	TYPE* alloc<TYPE>() { \
		TYPE* p; \
		if (recycle_bin_NAME.empty()) { \
			p = (TYPE*) (((char*) ::malloc(sizeof(T) + 1)) + 1); \
			heap_NAME.emplace_back(p); \
		} else { \
			p = recycle_bin_NAME.back(); \
			recycle_bin_NAME.pop_back(); \
		}\
		((Usage*) (((char*) p) - 1))->mark = Usage::Color::WHITE; \
		return p; \
	} \
	inline void destroy(TYPE* ptr) { \
		((Usage*) (((char*) ptr) - 1))->mark = Usage::Color::FREE; \
		recycle_bin_NAME.emplace_back(ptr); \
		ptr->~TYPE(); \
	}

#define DLANG__GC_SWEEP(TYPE, NAME) \
	for (auto* ptr : heap_NAME) { \
		 if (((Usage*) (((char*) ptr) - 1))->mark == Usage::Color::WHITE) \
		 	destroy(ptr); \
    }

	DLANG__GC_HEAP(Value, value);
	DLANG__GC_SPEC(Value, value);

	// Free items not in use
	void sweep() {
		// Sweep generic types
		for (int i = (int) generic_ptrs.size() - 1; i >= 0; i--) {
			Usage* u = (Usage*) (((char*) generic_ptrs[i]) - 1);
			if(u->mark == Usage::Color::WHITE) {
				generic_destructors[i].destroy(generic_ptrs[i]);
				free(u);
				generic_ptrs.erase(generic_ptrs.begin() + i);
				generic_destructors.erase(generic_destructors.begin() + i);
			}
		}

		DLANG__GC_SWEEP(Value, value);
	}
}
