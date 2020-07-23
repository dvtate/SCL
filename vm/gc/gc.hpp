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
    template <class T> struct Destructor {
		/**
		 * Calls type T destructor on given object passed via pointer
		 *
		 * @param obj - pointer to object to destroy
		 */
		virtual void destroy(T* obj) const {
			obj->~T();
		}
	};
	//generic finalizer that does nothing
	struct _Destructor {
		virtual void destroy(void *obj) const {}
	};

    // TODO paging, recycling, etc.
	std::deque<void*> generic_ptrs;
	std::deque<_Destructor> generic_destructors;

	// tracking data
	typedef struct Usage {
		// NOTE: this is misleading as this isn't a complete tricolor GC
		enum class Color : unsigned int {
			WHITE = 0,		// Candidate for GC
			GREY,			// Not a candidate for GC
			BLACK,			// TODO: tricolor tracing
			FREE,			// This object is in recycle bin
		} mark : 2;
	} Usage;

	// Tracing

	/// Mark items that are in use... gets called by Handle<>.mark()
	template<class T>
	inline void mark(T ptr) {
		((Usage*) (((char*) ptr) - 1))->mark = Usage::Color::GREY; // TODO tricolor
	}

	template<> mark<Handle<Value>>(Handle<Value> ptr);
	template<> mark<Value&>(Value& ptr);







	/// Construct GC'd object in place
	template <class T, class... Args>
	T* make(Args&&... args) {
		Usage* p = malloc(sizeof(T) + 1) + 1;
		p->mark = Usage::Color::WHITE; // TODO tricolor
		T* ret = (T*)((char*)p + 1);
		::new(ret) T(std::forward<Args>(args)...);
		generic_ptrs.emplace_back(ret);
		generic_destructors.emplace_back();
		const Destructor<T> destroy;
		std::memcpy(&generic_destructors.back(), &destroy, sizeof(destroy));
//		heap.emplace_back(p);
		return p;
	}

	// Free items not in use
	void sweep();

	void do_gc();
}






#endif //DLANG_GC_HPP
