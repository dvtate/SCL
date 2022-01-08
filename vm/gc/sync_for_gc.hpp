//
// Created by tate on 08/01/2022.
//

#ifndef SCL_SYNC_FOR_GC_HPP
#define SCL_SYNC_FOR_GC_HPP

#include <memory>

#include "../vm.hpp"

class SyncForGC {
	// Number of thread
	uint16_t counter;

	std::mutex counter_mtx;

	std::mutex gc_finished;

public:
	SyncForGC(uint16_t nthreads): counter(nthreads) {
		this->gc_finished.lock();
	}

	void action(Runtime& rt) {
		{
			std::lock_guard<std::mutex> guard(this->counter_mtx);

			// If all other threads are waiting on gc
			if (--this->counter) {
				// Do gc
				rt.vm->mark();
				rt.vm->gc.sweep();

				// Unlock other threads
				this->gc_finished.unlock();
				return;
			}
		}

		// TODO there should be a more efficent way to do this
		// make processes wait until gc finished
		this->gc_finished.lock();
		this->gc_finished.unlock();
	}
};

// When the VM is running multiple threads, they have to all sync before we can GC
// This RT message makes the thread hang until the GC is finished
class SyncForGCMsg : public RTMessage {
	std::shared_ptr<SyncForGC> mgr;
public:
	explicit SyncForGCMsg(std::shared_ptr<SyncForGC> mgr): mgr(std::move(mgr)) {}

	// Run once for each thread
	virtual void action(Runtime& rt) override {
		mgr->action(rt);
	}
};


#endif //SCL_SYNC_FOR_GC_HPP
