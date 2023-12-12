//
// Created by tate on 12/06/2021.
//

#include "../value.hpp"
#include "../closure.hpp"
#include "../lambda_return.hpp"

#include "gc.hpp"

/// Free items not in use
void GarbageCollector::sweep() {
	// Sweep the segregated heaps
	// TODO parallelize?
	for (auto* ptr : this->_heap_value) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
		 	destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
    }
	for (auto* ptr : this->_heap_list) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
			destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
	}
	for (auto* ptr : this->_heap_obj) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
			destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
	}
	for (auto* ptr : this->_heap_closure) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
			destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
	}
	for (auto* ptr : this->_heap_nfn) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
			destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
	}
	for (auto* ptr : this->_heap_lamret) {
		auto* u = (GarbageCollector::UsageColor*) (((char*) ptr) - 1);
		if (*u == GarbageCollector::UsageColor::WHITE)
			destroy(ptr);
		else if (*u == GarbageCollector::UsageColor::GREY)
			*u = GarbageCollector::UsageColor::WHITE;
	}

	// Sweep generic pointers
	// TODO optimize, use <algorithm>
	// TODO generational + moving or whatever
	std::vector<std::size_t> keepers;
	keepers.reserve(this->generic_ptrs.size() >> 1);
	for (long i = (long) this->generic_ptrs.size() - 1; i >= 0; i--) {
		auto* u = (UsageColor*) (((char*) this->generic_ptrs[i]) - 1);
		if (*u == UsageColor::WHITE) {
			this->generic_destructors[i].destroy(u + 1);
			free(u);
		} else {
			*u = UsageColor::WHITE;
			keepers.emplace_back(i);
		}
	}

	// Filter only keepers
	std::deque<void*> gps;
	std::deque<_Destructor> gds;
	std::swap(gps, this->generic_ptrs);
	std::swap(gds, this->generic_destructors);
	for (const auto i : keepers) {
		this->generic_ptrs.emplace_back(gps[i]);
		this->generic_destructors.emplace_back(gds[i]);
	}

	this->last_gc_size = this->size();
}

/// Rough heap size
[[nodiscard]] std::size_t GarbageCollector::size() const {
	return this->generic_ptrs.size()
	+ this->_heap_value.size()
	+ this->_heap_list.size()
	+ this->_heap_obj.size()
	+ this->_heap_nfn.size()
	+ this->_heap_closure.size()
	+ this->_heap_lamret.size();
	// Not going to factor in the sizes of the recycle-bins
}

/// do we need to gc?
// TODO change algorithm: if size > threshold or size > last_gc_size
[[nodiscard]] bool GarbageCollector::need_gc() const {
	static constexpr unsigned THRESHOLD = 1 << 14;
	const auto size = this->size();
	return size > 10000 && size - this->last_gc_size > THRESHOLD;
}

/// print heap summary
void GarbageCollector::debug() {
	std::cout <<"Generic: \n\tsize: " <<this->generic_ptrs.size()
			  <<"\nValue: \n\tSize: " <<this->_heap_value.size()
			  <<"\n\tRecyclable: " <<this->_recycle_value.size()
			  <<"\nValueTypes::obj_t: \n\tSize: " <<this->_heap_obj.size()
			  <<"\n\tRecyclable: " <<this->_recycle_obj.size()
			  <<"\nNativeFunction: \n\tSize: " <<this->_heap_nfn.size()
			  <<"\n\tRecyclable: " <<this->_recycle_nfn.size()
			  <<"\nClosure: \n\tSize: " <<this->_heap_closure.size()
			  <<"\n\tRecyclable: " <<this->_recycle_closure.size()
			  <<"\nValueTypes::list_t: \n\tSize: " <<this->_heap_list.size()
			  <<"\n\tRecyclable: " <<this->_recycle_list.size()
			  <<"\nLambdaReturn_NF: \n\tSize: " <<this->_heap_lamret.size()
			  <<"\n\tRecyclable: " <<this->_recycle_lamret.size()
			  <<std::endl;
}

// Specializations for given type and it's corresponding heap/recycle name
#define SCL__GC_SPEC(TYPE, NAME) \
	template<> \
	TYPE* GC::alloc<TYPE>(GarbageCollector& gc) { \
		TYPE* p; \
		if (gc._recycle_##NAME.empty()) { \
			p = (TYPE*) (((char*)::malloc(sizeof(TYPE) + 1)) + 1); \
			gc._heap_##NAME.emplace_back(p); \
		} else { \
			p = gc._recycle_##NAME.back(); \
			/* std::cout <<"recycled "<<#TYPE <<": " <<(void*) p <<std::endl;*/\
			gc._recycle_##NAME.pop_back(); \
		} \
		*(((char*) p) - 1) = (char) GarbageCollector::UsageColor::WHITE; \
		return p; \
	} \
	void GarbageCollector::destroy(TYPE* ptr) { \
		/*std::cout <<"destroy: " <<#TYPE #NAME <<(void*)ptr <<std::endl;*/ \
		*((GarbageCollector::UsageColor*) (((char*) ptr) - 1)) = GarbageCollector::UsageColor::FREE; \
		this->_recycle_##NAME.emplace_back(ptr); \
		ptr->~TYPE(); \
	}

using ValueTypes::obj_t;
using ValueTypes::list_t;
SCL__GC_SPEC(Value, value);
SCL__GC_SPEC(obj_t, obj);
SCL__GC_SPEC(list_t, list);
SCL__GC_SPEC(NativeFunction, nfn);
SCL__GC_SPEC(NativeClosure, nfc);
SCL__GC_SPEC(Closure, closure);
SCL__GC_SPEC(LambdaReturnNativeFn, lamret);
