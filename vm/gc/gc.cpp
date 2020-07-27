//
// Created by tate on 30-04-20.
//

#include "../value.hpp"
#include "../vm.hpp"
#include "../closure.hpp"
#include "gc.hpp"

namespace GC {
	unsigned long last_gc_size = 0;

	std::deque<void *> generic_ptrs;
	std::deque<_Destructor> generic_destructors;

// Declare tracking containers
#define DLANG__GC_HEAP_NAME(NAME) heap_##NAME
#define DLANG__GC_RECYCLE_NAME(NAME) recycle_bin_##NAME
#define DLANG__GC_HEAP(TYPE, NAME) \
	std::vector<TYPE*> DLANG__GC_HEAP_NAME(NAME); \
	std::vector<TYPE*> DLANG__GC_RECYCLE_NAME(NAME);

// Function template specializations for given type
#define DLANG__GC_SPEC(TYPE, NAME) \
	template<> \
	TYPE* alloc<TYPE>() { \
		TYPE* p; \
		if (recycle_bin_##NAME.empty()) { \
			p = (TYPE*) (((char*) ::malloc(sizeof(TYPE) + 1)) + 1); \
			heap_##NAME.emplace_back(p); \
		} else { \
			p = recycle_bin_##NAME.back(); \
			/* std::cout <<"recycled "<<#TYPE <<": " <<(void*) p <<std::endl;*/\
			recycle_bin_##NAME.pop_back(); \
		} \
		((Usage*) (((char*) p) - 1))->mark = Usage::Color::WHITE; \
		return p; \
	} \
	inline void destroy(TYPE* ptr) { \
		/*std::cout <<"destroy: " <<#TYPE #NAME <<(void*)ptr <<std::endl;*/ \
		((Usage*) (((char*) ptr) - 1))->mark = Usage::Color::FREE; \
		recycle_bin_##NAME.emplace_back(ptr); \
		ptr->~TYPE(); \
	}

#define DLANG__GC_SWEEP(NAME) \
	for (auto* ptr : DLANG__GC_HEAP_NAME(NAME)) { \
		auto* u = (Usage*) (((char*) ptr) - 1); \
		if (u->mark == Usage::Color::WHITE) \
		 	destroy(ptr); \
		else \
			u->mark = Usage::Color::WHITE;\
    }

	DLANG__GC_HEAP(Value, value);
	DLANG__GC_SPEC(Value, value);

	using ValueTypes::list_t;
	using ValueTypes::obj_t;
	DLANG__GC_HEAP(list_t, value_list);
	DLANG__GC_SPEC(list_t, value_list);

	DLANG__GC_HEAP(obj_t, value_obj);
	DLANG__GC_SPEC(obj_t, value_obj);

	DLANG__GC_HEAP(NativeFunction, value_nfn);
	DLANG__GC_SPEC(NativeFunction, value_nfn);

	DLANG__GC_HEAP(Closure, value_closure);
	DLANG__GC_SPEC(Closure, value_closure);

	// Free items not in use
	void sweep() {
		// Sweep generic types
		for (int i = (int) generic_ptrs.size() - 1; i >= 0; i--) {
			Usage* u = (Usage*) (((char*) generic_ptrs[i]) - 1);
//			std::cout <<"generic: color: " <<(int) u->mark <<std::endl;
			if(u->mark == Usage::Color::WHITE) {
//				std::cout <<"Destroy generic\n";
				generic_destructors[i].destroy(generic_ptrs[i]);
				free(u);
				generic_ptrs.erase(generic_ptrs.begin() + i);
				generic_destructors.erase(generic_destructors.begin() + i);
			} else {
				u->mark = Usage::Color::WHITE;
			}
		}

		// Sweep Segregated storage
		DLANG__GC_SWEEP(value);
		DLANG__GC_SWEEP(value_list);
		DLANG__GC_SWEEP(value_obj);
		DLANG__GC_SWEEP(value_nfn);
		DLANG__GC_SWEEP(value_closure);
	}

	unsigned long size() {
		// Subtract the recycle bin contents
		const auto ret = generic_ptrs.size();
			+ DLANG__GC_HEAP_NAME(value).size()
			+ DLANG__GC_HEAP_NAME(value_list).size()
			+ DLANG__GC_HEAP_NAME(value_obj).size()
			+ DLANG__GC_HEAP_NAME(value_nfn).size()
			+ DLANG__GC_HEAP_NAME(value_closure).size()
			- DLANG__GC_RECYCLE_NAME(value).size()
			- DLANG__GC_RECYCLE_NAME(value_list).size()
			- DLANG__GC_RECYCLE_NAME(value_obj).size()
			- DLANG__GC_RECYCLE_NAME(value_nfn).size()
			- DLANG__GC_RECYCLE_NAME(value_closure).size();

//		std::cout <<ret <<std::endl;
		return ret;
	}
}
