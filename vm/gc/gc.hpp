//
// Created by tate on 12/06/2021.
//

#ifndef SCL_GC_2_HPP
#define SCL_GC_2_HPP

#include <vector>
#include <memory>
#include <cstdlib>
#include <cstring>

#include "../value_types.hpp"

// Forward declarations
class Value;
class Closure;
class LambdaReturnNativeFn;
class GarbageCollector;
namespace GC {
	template <class T>	T* alloc(GarbageCollector&);
	template <class T>	T* static_alloc();
}

// GC instance variables
class GarbageCollector {
	/// Needed to track destructor for object because &Type::~Type is not allowed
	/// This seems like a hack but it works ig...
	template<class T>
	struct Destructor {
		/**
		 * Calls type T destructor on given object passed via pointer
		 *
		 * @param obj - pointer to object to destroy
		 */
		virtual void destroy(T* obj) const {
			obj->~T();
		}
	};
	// generic finalizer that does nothing
	struct _Destructor {
		virtual void destroy(void* obj) const {
			(void)(obj); // Suppress unused parameter warning
		}
	};
	static_assert(sizeof(Destructor<ValueTypes::obj_t>) == sizeof(_Destructor), "Destructor wrong size");
	static_assert(sizeof(Destructor<ValueTypes::list_t>) == sizeof(_Destructor), "Destructor wrong size");

	std::deque<void*> generic_ptrs;
	std::deque<_Destructor> generic_destructors;
	std::vector<void*> static_objects; // any reason to track these?

	std::size_t last_gc_size{0};

public:
	// Tracking data
	enum class UsageColor : char {
		WHITE = 0,        // Candidate for GC
		GREY,            // Not a candidate for GC
		BLACK,            // TODO: tricolor tracing
		FREE,            // This object is in recycle bin
	};

	// Free items not in use
	void sweep();

	// Rough heap size
	[[nodiscard]] std::size_t size() const;

	// do we need to gc?
	[[nodiscard]] bool need_gc() const;

	// Print heap summary
	void debug();

	// Allocate an object that isn't garbage collected or tracked
	template <class T>
	T* static_alloc() {
		T* ret = GC::static_alloc<T>();
		this->static_objects.emplace_back(ret);
		return ret;
	}

	/// Allocate a garbage collected object
	// Note that C++ only allows explicit template specialization at
	// namespace scope so we have to do this BS
	template <class T>
	T* alloc() {
		return GC::alloc<T>(*this);
	}

	// Make alloc a friend
	template <class T> friend
	T* GC::alloc(GarbageCollector&);

private:
	// Segregated Heaps
	std::vector<Value*> _heap_value;
	std::vector<Value*> _recycle_value;
	std::vector<ValueTypes::list_t*> _heap_list;
	std::vector<ValueTypes::list_t*> _recycle_list;
	std::vector<ValueTypes::obj_t*> _heap_obj;
	std::vector<ValueTypes::obj_t*> _recycle_obj;
	std::vector<NativeFunction*> _heap_nfn;
	std::vector<NativeFunction*> _recycle_nfn;
	std::vector<Closure*> _heap_closure;
	std::vector<Closure*> _recycle_closure;
	std::vector<LambdaReturnNativeFn*> _heap_lamret;
	std::vector<LambdaReturnNativeFn*> _recycle_lamret;

	// Destructor proxies
	void destroy(Value*);
	void destroy(ValueTypes::list_t*);
	void destroy(ValueTypes::obj_t*);
	void destroy(NativeFunction*);
	void destroy(Closure*);
	void destroy(LambdaReturnNativeFn*);
};


namespace GC {
	/// Mark items that are in use...
	/// Note: for complex types you should also call mark on it's children
	/// 	This is done by overloading the mark method for particular classes
	/// \param ptr - address
	/// \return false if already been marked true if not
	inline bool mark(void* ptr) {
		auto* p = ((GarbageCollector::UsageColor*) (((char*) ptr) - 1));
		if (*p != GarbageCollector::UsageColor::GREY) {
			*p = GarbageCollector::UsageColor::GREY;
			return true;
		} else {
			return false;
		}
	}

	extern void mark(Value&);
	extern void mark(Value*);
	extern void mark(ValueTypes::list_t&);
	extern void mark(ValueTypes::list_t*);
	extern void mark(ValueTypes::obj_t&);
	extern void mark(ValueTypes::obj_t*);
	extern void mark(NativeFunction&);
	extern void mark(NativeFunction*);
	extern void mark(Closure&);
	extern void mark(Closure*);
	extern void mark(LambdaReturnNativeFn&);
	extern void mark(LambdaReturnNativeFn*);

	// Allocate an object that isn't garbage collected or tracked
	template <class T>
	T* static_alloc() {
		auto* p = (GarbageCollector::UsageColor*) ((char*) ::malloc(sizeof(T) + 1));
		*p = GarbageCollector::UsageColor::GREY;
		T* ret = (T*)(((char*)p) + 1);
		return ret;
	}

	// Allocate a garbage collected object
	template <class T>
	T* alloc(GarbageCollector& gc) {
		auto* p = (GarbageCollector::UsageColor*) ((char*) ::malloc(sizeof(T) + 1));
		*p = GarbageCollector::UsageColor::WHITE; // TODO tricolor
		T* ret = (T*)(((char*)p) + 1);
		gc.generic_ptrs.emplace_back(ret);
		gc.generic_destructors.emplace_back();
		// TODO this works but there should be a less ghetto way to do this...
		const GarbageCollector::Destructor<T> destroy;
		std::memcpy(&gc.generic_destructors.back(), &destroy, sizeof(GarbageCollector::_Destructor));
		return ret;
	}

	// template specializations for alloc
	template<> Value* alloc<Value>(GarbageCollector&);
	template<> ValueTypes::list_t* alloc<ValueTypes::list_t>(GarbageCollector&);
	template<> ValueTypes::obj_t* alloc<ValueTypes::obj_t>(GarbageCollector&);
	template<> NativeFunction* alloc<NativeFunction>(GarbageCollector&);
	template<> Closure* alloc<Closure>(GarbageCollector&);
	template<> LambdaReturnNativeFn* alloc<LambdaReturnNativeFn>(GarbageCollector&);
}

#endif //SCL_GC_2_HPP
