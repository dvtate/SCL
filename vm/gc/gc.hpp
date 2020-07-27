////
//// Created by tate on 30-04-20.
////

#ifndef DLANG_GC_HPP
#define DLANG_GC_HPP

#include <atomic>
#include <vector>
#include <memory>
#include <deque>

#include <cstdlib>
#include <cstring> // memcpy

#include "../value_types.hpp"

/* TODO
 * Generic pointer types
 * Space tracking
 * Load factor
 * Tricolor tracing
 * Concurrent tracing
 *
 * DONE
 * - naive marking
 * -
 *
 * MAYBE
 * - paging
 * -
 */

/**
 * This rough draft is a naive stw mark and sweep gc
 * Features:
 * - Segregated storage for first-party types
 * -
 * - Recyling
 * -
 */


//
namespace GC {

	/// Needed to track destructor for object because &Type::~Type is not allowed
	/// This seems like a hack but it works ig...
	template<class T>
	struct Destructor {
		/**
		 * Calls type T destructor on given object passed via pointer
		 *
		 * @param obj - pointer to object to destroy
		 */
		virtual void destroy(T *obj) const {
			obj->~T();
		}
	};
	//generic finalizer that does nothing
	struct _Destructor {
		virtual void destroy(void* obj) const {}
	};

	// Track 3rd party types
	// TODO paging, recycling, etc.
	extern std::deque<void *> generic_ptrs;
	extern std::deque<_Destructor> generic_destructors;


	// tracking data
	typedef struct Usage {
		// NOTE: this is misleading as this isn't a complete tricolor GC
		enum class Color : unsigned int {
			WHITE = 0,        // Candidate for GC
			GREY,            // Not a candidate for GC
			BLACK,            // TODO: tricolor tracing
			FREE,            // This object is in recycle bin
		} mark: 2;
	} Usage;

	// Tracing

	/// Mark items that are in use...
	/// Note: for complex types you should also call mark on it's children
	/// 	This is best done by overloading the
	inline bool mark(void* ptr) {
		auto* p = ((Usage *) (((char *) ptr) - 1));
		if (p->mark != Usage::Color::GREY) {
			p->mark = Usage::Color::GREY;
			return true;
		} else {
			return false;
		}
	}

		 template <class T>
	T* alloc() {
		Usage* p = (Usage*) (((char*) ::malloc(sizeof(T) + 1)) + 1);
		p->mark = Usage::Color::WHITE; // TODO tricolor
		T* ret = (T*)(((char*)p) + 1);
		generic_ptrs.emplace_back(ret);
		generic_destructors.emplace_back();
		const Destructor<T> destroy;
		std::memcpy(&generic_destructors.back(), &destroy, sizeof(destroy));
		return ret;
	}

	// Free items not in use
	void sweep();

	// Mark + sweep
	void do_gc();

	// How big is heap?
	unsigned long size();

	// Threshold heap growth for GC
	constexpr unsigned long THRESHOLD = 1024;

	//
	extern unsigned long last_gc_size;
}


// Specializations for common internal types
class Value;
class Closure;

namespace GC {

#define DLANG__GC_DECLS(TYPE) \
	template<> TYPE* alloc<TYPE>(void); \
	extern void mark(TYPE&); \
	extern void mark(TYPE*);

	DLANG__GC_DECLS(Value);
	DLANG__GC_DECLS(ValueTypes::list_t);
	DLANG__GC_DECLS(ValueTypes::obj_t);
	DLANG__GC_DECLS(NativeFunction);
	DLANG__GC_DECLS(Closure);

}

#endif //DLANG_GC_HPP
