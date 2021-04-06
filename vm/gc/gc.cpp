//
// Created by tate on 30-04-20.
//

#include "../value.hpp"
#include "../vm.hpp"
#include "../closure.hpp"
#include "../lambda_return.hpp"
#include "gc.hpp"


namespace GC {
	unsigned long last_gc_size = 0;

	std::deque<void*> generic_ptrs;
	std::deque<_Destructor> generic_destructors;
	std::vector<void*> static_objects;

// Declare tracking containers
#define SCL__GC_HEAP_NAME(NAME) heap_##NAME
#define SCL__GC_RECYCLE_NAME(NAME) recycle_bin_##NAME
#define SCL__GC_HEAP(TYPE, NAME) \
	std::vector<TYPE*> SCL__GC_HEAP_NAME(NAME); \
	std::vector<TYPE*> SCL__GC_RECYCLE_NAME(NAME);

// Function template specializations for given type
#define SCL__GC_SPEC(TYPE, NAME) \
	template<> \
	TYPE* alloc<TYPE>() { \
		TYPE* p; \
		if (recycle_bin_##NAME.empty()) { \
			p = (TYPE*) (((char*)::malloc(sizeof(TYPE) + 1)) + 1); \
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

#define SCL__GC_SWEEP(NAME) \
	for (auto* ptr : SCL__GC_HEAP_NAME(NAME)) { \
		auto* u = (Usage*) (((char*) ptr) - 1); \
		if (u->mark == Usage::Color::WHITE) \
		 	destroy(ptr); \
		else if (u->mark == Usage::Color::GREY) \
			u->mark = Usage::Color::WHITE;\
    }

	SCL__GC_HEAP(Value, value);
	SCL__GC_SPEC(Value, value);

	using ValueTypes::list_t;
	using ValueTypes::obj_t;
	SCL__GC_HEAP(list_t, value_list);
	SCL__GC_SPEC(list_t, value_list);

	SCL__GC_HEAP(obj_t, value_obj);
	SCL__GC_SPEC(obj_t, value_obj);

	SCL__GC_HEAP(NativeFunction, value_nfn);
	SCL__GC_SPEC(NativeFunction, value_nfn);

	SCL__GC_HEAP(Closure, value_closure);
	SCL__GC_SPEC(Closure, value_closure);

	SCL__GC_HEAP(LambdaReturnNativeFn, value_lamret_nf);
	SCL__GC_SPEC(LambdaReturnNativeFn, value_lamret_nf);

	// Free items not in use
	void sweep() {
		// Sweep Segregated storage
		SCL__GC_SWEEP(value);
		SCL__GC_SWEEP(value_list);
		SCL__GC_SWEEP(value_obj);
		SCL__GC_SWEEP(value_nfn);
		SCL__GC_SWEEP(value_closure);
		SCL__GC_SWEEP(value_lamret_nf);

		// Sweep generic types.. todo: optimize
		std::vector<unsigned long> keepers;
		keepers.reserve(generic_ptrs.size() / 2);
		for (int i = (int) generic_ptrs.size() - 1; i >= 0; i--) {
			Usage* u = (Usage*) (((char*) generic_ptrs[i]) - 1);
//			std::cout <<"generic: color: " <<(int) u->mark <<std::endl;
			if(u->mark == Usage::Color::WHITE) {
//				std::cout <<"Destroy generic\n";
				generic_destructors[i].destroy(generic_ptrs[i]);
				free(u);
			} else {
				u->mark = Usage::Color::WHITE;
				keepers.emplace_back(i);
			}
		}
		if (!keepers.empty()) {
			std::deque<void *> gps;
			std::deque<_Destructor> gds;
			std::swap(gps, generic_ptrs);
			std::swap(gds, generic_destructors);
			for (auto i : keepers) {
				generic_ptrs.emplace_back(gps[i]);
				generic_destructors.emplace_back(gds[i]);
			}
		}
	}

	unsigned long size() {
		// Subtract the recycle bin contents
		return generic_ptrs.size()
			+ SCL__GC_HEAP_NAME(value).size()
			+ SCL__GC_HEAP_NAME(value_list).size()
			+ SCL__GC_HEAP_NAME(value_obj).size()
			+ SCL__GC_HEAP_NAME(value_nfn).size()
			+ SCL__GC_HEAP_NAME(value_closure).size()
			+ SCL__GC_HEAP_NAME(value_lamret_nf).size()
			- SCL__GC_RECYCLE_NAME(value).size()
			- SCL__GC_RECYCLE_NAME(value_list).size()
			- SCL__GC_RECYCLE_NAME(value_obj).size()
			- SCL__GC_RECYCLE_NAME(value_nfn).size()
			- SCL__GC_RECYCLE_NAME(value_closure).size()
			- SCL__GC_RECYCLE_NAME(value_lamret_nf).size();
	}

	void print_summary() {
		std::cout <<"Generic: \n\tsize: " << generic_ptrs.size()
			<<"\nValue: \n\tSize: " <<SCL__GC_HEAP_NAME(value).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value).size()
			<<"\nValueTypes::obj_t: \n\tSize: " <<SCL__GC_HEAP_NAME(value_obj).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value_obj).size()
			<<"\nNativeFunction: \n\tSize: " <<SCL__GC_HEAP_NAME(value_nfn).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value_nfn).size()
			<<"\nClosure: \n\tSize: " <<SCL__GC_HEAP_NAME(value_closure).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value_closure).size()
			<<"\nValueTypes::list_t: \n\tSize: " <<SCL__GC_HEAP_NAME(value_list).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value_list).size()
			<<"\nLambdaReturn_NF: \n\tSize: " <<SCL__GC_HEAP_NAME(value_lamret_nf).size()
			<<"\n\tRecyclable: " <<SCL__GC_RECYCLE_NAME(value_lamret_nf).size()
			<<std::endl;
	}
}
