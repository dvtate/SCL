//
// Created by tate on 08/01/2022.
//

#ifndef SCL_SYNC_FOR_GC_HPP
#define SCL_SYNC_FOR_GC_HPP

#include <memory>

#include "../vm.hpp"

class GCBarrier {
	// Number of thread
	uint16_t counter;

	std::mutex counter_mtx;

	std::mutex gc_finished;

public:
	explicit GCBarrier(const uint16_t nthreads): counter(nthreads) {
		this->gc_finished.lock();
	}

	void action(Runtime& rt) {
		{
			std::lock_guard<std::mutex> guard{this->counter_mtx};

//			std::cout <<"counter lock\n";

			// If all other threads are waiting on gc
			if (--this->counter) {
				// Do gc
				rt.vm->mark();
				rt.vm->gc.sweep();
//				std::cout <<"did gc\n";

				// Unlock other threads
				this->gc_finished.unlock();
				return;
			}
		}

		// TODO there should be a more efficent way to do this
		// make processes wait until gc finished
		this->gc_finished.lock();
		this->gc_finished.unlock();
//		std::cout <<"gc finished\n";
	}
};

// When the VM is running multiple threads, they have to all sync before we can GC
// This RT message makes the thread hang until the GC is finished
class SyncForGCMsg : public RTMessage {
	std::shared_ptr<GCBarrier> mgr;
public:
	explicit SyncForGCMsg(std::shared_ptr<GCBarrier> mgr): mgr(std::move(mgr)) {}

	// Run once for each thread
	virtual void action(Runtime& rt) override {
		mgr->action(rt);
	}
};

#endif //SCL_SYNC_FOR_GC_HPP
