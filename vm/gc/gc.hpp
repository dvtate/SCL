////
//// Created by tate on 30-04-20.
////

#ifndef DLANG_GC_HPP
#define DLANG_GC_HPP

#include <atomic>
#include <vector>
#include <memory>
#include <cstdlib>

#define DLANG__GC_DECL(TYPE, NAME) \
	std::vector<TYPE> heap_NAME;

// Function template specializations for given type
// TODO tricolor
//
#define DLANG__GC_SPEC(TYPE, NAME) \
	template<class... Args> \
	TYPE* make<TYPE>(Args&& ...args) { \
		void* p = malloc(sizeof(T) + 1) + 1; \
		::new(p) T(std::forward<Args>(args)...); \
		heap.emplace_back(p); \
		return p; \
	} \
	template<> \
	inline void mark(T* ptr) { \
		((Usage*) (((char*) ptr) - 1))->mark = Usage::Color::GREY; \
	} \


//
namespace GC {
	// TODO use segregated storage, root nodes, space tracking, etc.
	extern std::vector<void*> heap_void;

	DLANG__GC_DECL(Handle<Value>, value);
	DLANG__GC_SPEC(Handle<Value>, value);

	struct Usage {

		// NOTE: this is misleading as this isn't a complete tricolor GC
		// TODO tricolor garbage collector
		enum class Color : unsigned int {
			WHITE = 0,		// Candidate for GC
			GREY,			// Not a candidate for GC
			BLACK,			// WiP
		} mark : 2;
	};

	// Construct GC'd object in place
	template <class T, class... Args>
	T* make(Args&&... args) {
		void* p = malloc(sizeof(T) + 1) + 1;
		::new(p) T(std::forward<Args>(args)...);
		heap.emplace_back(p);
		return p;
	}

	// Mark items that are in use... gets called by Handle<>.mark()
	template<typename T>
	inline void mark(T* ptr) {
		((Usage*) (((char*) ptr) - 1))->mark = Usage::Color::GREY; // TODO tricolor
	}

	// Unspecialzed === dont use GC as we don't have a segregated type for it...
	// TODO we still want to support tracing for user-defined types (ie - language extensions)
	//  can do this with a distinct separate usage +
	// Construct GC'd object in place
	template <class T, class... Args>
	T* make(Args&&... args) {
		return new T(std::forward<Args>(args)...);
		return p;
	}
	// Mark items that are in use... gets called by Handle<>.mark()
	template<typename T>
	inline void mark(T* ptr) {
		return;
	}

	// Free items not in use
	void sweep() {
		for (void* ptr : heap) {
			Usage* p = (Usage*) (((char *) p) - 1);
			if (p->mark == Usage::Color::WHITE) {
				::operator delete(ptr);
				free(p);
			}
		}
	}
}






#endif //DLANG_GC_HPP
