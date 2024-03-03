//
// Created by tate on 17-05-20.
//

#ifndef SCL_VM_HPP
#define SCL_VM_HPP

#include <iostream>
#include <thread>
#include <mutex>

#include <vector>
#include <queue>
#include <cstdint>
#include <list>

#include "../debug.hpp"
#include "closure.hpp"
#include "bc/exec_bc_instr.hpp"
#include "literal.hpp"
#include "value.hpp"
#include "global_ids.hpp"
#include "bc/fault_table.hpp"
#include "operators/internal_tools.hpp"
#include "gc/gc.hpp"

// Interrelated classes
class Frame;
class Runtime;
class VM;

/// abstract type used for ITC/IPC/Synchronization
class RTMessage {
public:
	virtual ~RTMessage() { }
	virtual void action(Runtime&) = 0;
	virtual void mark() { }
};

// TODO
class SyncCallStack  {
public:
	std::vector<std::shared_ptr<Frame>> stack;

	SyncCallStack() {
		this->stack.reserve(32);
	}

	void mark();

	void throw_error(Value thrown);
};


/// different threads running on same process
class Runtime {
public:
	VM* vm;

	/// Currently running thread
	std::shared_ptr<SyncCallStack> running;

	/// Threads to execute [stack]
	std::vector<std::shared_ptr<SyncCallStack>> active;

	/// Threads that still have pending actions
	std::vector<std::shared_ptr<SyncCallStack>> undead;

	/// Thread safety as messages can come from different procs/ISR's
	std::mutex msg_queue_mtx;

	explicit Runtime(VM* vm): vm(vm) { }

	/// pushes msg onto msg queue
	void recv_msg(RTMessage* msg) {
		std::lock_guard<std::mutex> guard{this->msg_queue_mtx};
		this->_msg_queue.emplace_back(msg);
		SCL_DEBUG_MSG("VM:RT: msg received\n");
	}

	/// clears msg queue and returns old contents
	/// \remark caller frees
	std::vector<RTMessage*> clear_msg_queue() {
		std::vector<RTMessage*> cpy = {};
		std::lock_guard<std::mutex> m{this->msg_queue_mtx};
		std::swap(cpy, this->_msg_queue);
		SCL_DEBUG_MSG("VM:RT: cleared msg queue\n");
		return cpy;
	}

	// TODO these memeber functions need to be renamed/deleted/refactored

	/// Replaces running with a new thread
	void spawn_thread(){
		auto rcs = this->running;
		this->freeze_running();
		this->active.emplace_back(rcs);
		this->running = std::make_shared<SyncCallStack>();
		this->undead.emplace_back(this->running);
	}

	void freeze_running() {
		if (this->active.empty()) {
			this->running = nullptr;
			return;
		}
		this->running = this->active.back();
		this->active.pop_back();
		SCL_DEBUG_MSG("VM:RT: froze running CallStack\n");
	}

	void freeze_active(const std::shared_ptr<SyncCallStack>& cs) {
		for (auto it = this->active.begin(); it < this->active.end(); it += 1)
			if (*it == cs) {
				this->active.erase(it);
				return;
			}

		SCL_DEBUG_MSG("VM:RT: froze pending CallStack\n");
	}

	/// removes call stack from undead tracker
	void kill(const std::shared_ptr<SyncCallStack>& cs){
		// TODO replace undead with a std::set or sth
		for (auto it = this->undead.begin(); it < this->undead.end(); it += 1)
			if (*it == cs) {
				this->undead.erase(it);
				return;
			}
		SCL_DEBUG_MSG("VM:RT: killed CallStack\n");
	}

	void kill_running(){
		auto cs = this->running;
		this->freeze_running();
		this->kill(cs);
	}

	void set_active(std::shared_ptr<SyncCallStack>& cs) {
		// if stack target not in active stacks, put there
		if (this->running != cs &&
			// TODO replace this->active with a set or sth
			std::find(this->active.begin(), this->active.end(), cs) == this->active.end()) \
		{
			SCL_DEBUG_MSG("VM:RT: queued stack..\n");
			this->active.emplace_back(cs);
		}
	}

	// Process entry point
	void run();

private:
	// mutex guards on r/w
	// NOTE: treated as queue because all msgs read at once in FIFO order
	std::vector<RTMessage*> _msg_queue;

public:
	/// GC mark
	void mark() {
		if (this->running) {
			this->running->mark();
		}
		for (auto sp : this->undead) {
			sp->mark();
		}
	
		// Some messages have gc'd properties even though they're not gc'd
		std::lock_guard<std::mutex> m{this->msg_queue_mtx};
		for (auto* msg : this->_msg_queue)
			msg->mark();
	}

};

class VM {
public:
	/// Literals from the binary
	std::vector<Literal> literals;

	/// Initial user process
	std::shared_ptr<Runtime> main_thread;

	/// Child Processes
	std::list<std::shared_ptr<Runtime>> processes;

	/// Used for generating errors
	FaultTable* fault_table{nullptr};

	/// Stream of bytecode to execute
	std::istream& bytecode_source;

	/// One garbage collector shared by all the threads.
	GarbageCollector gc;

	VM(std::vector<Literal> lit_header, const std::vector<std::string>&  argv, std::istream& bytecode_source);
	~VM() {
		delete fault_table;
	}

	void run();

	void mark() {
		// TODO these shouldn't have to be marked
		for (auto& l : this->literals)
			l.mark();
		this->main_thread->mark();
		for (auto& sp : this->processes)
			sp->mark();
	}
};

class Frame {
public:
	Runtime* rt;

	/// Policy when error occurs
	Value* error_handler{nullptr};

	/// closure we're running
	Closure closure;

	/// index of bytecode read head
	uint_fast32_t pos;

	/// values we're working with
	std::vector<Value> eval_stack;

	Frame(Runtime* rt, Closure body, unsigned int pos = 0, std::vector<Value> eval_stack = {}):
			rt(rt), closure(std::move(body)), pos(pos), eval_stack(std::move(eval_stack))
	{}

	~Frame() {}

	/// run a single bytecode instruction and return
	inline bool tick() {
		if (pos < this->closure.body->size()) {
			exec_bc_instr(*this, (*this->closure.body)[this->pos++]);
			return false;
		}
		return true;
	}

	/// GC mark
	void mark() {
		// Mark Definition
		GC::mark(this->closure);

		// Mark stack
		for (Value& v : this->eval_stack)
			GC::mark(v);

		// Mark Error handler
		if (this->error_handler)
			GC::mark(this->error_handler);
	}

	/// Instanitiate object in place
	template<class T, class ... Args>
	[[nodiscard]] T* gc_make(Args&& ... args) {
		return ::new(this->rt->vm->gc.alloc<T>()) T(args...);
	}
};

// GC tracing
namespace GC {
	inline void mark(RTMessage& msg) {
		msg.mark();
	}
	inline void mark(RTMessage* msg) {
		if (mark((void*) msg))
			msg->mark();
	}
}
/* Original plan:
	Making calls:
	- on sync call, new Frame is pushed onto running.call_stack()
	- on async call, new thread is created and pushed to top of active stack
	- on parallel call, new Runtime is pushed onto VM.processes()

	Sync Detailed Operation:
	- on sync call: new Frame is pushed onto Runtime.running
    - if we reach end of a function:
    	- set Runtime.running = Runtime.active.back() and then pop Runtime.active.pop_back()
    	- thread will be revived if/when function eventually returns
	- when o() called:
 		- o is an id that corresponds to a native functor that when run,
 			creates a RTMessage Object and sends it to Runtime.recv_msg()
 		- when RTMessage is processed in between instruction ticks
 			- if msg.targeted_stack (shared_ptr<SyncCallStack) isn't in Runtime.running/active
 				- push it onto back of active stack
			- if msg.targeted_closure (Closure*) isn't in msg.targeted_stack
				- throw error/crash
			- pop items off msg.targeted_stack until reaching targeted_closure
			- pop off targeted_closure from stack too
			- push msg.return_value onto Frame.eval_stack
			- process next RTMessage(s)
			- go back to executing instructions

 	Async Operation:
 	- on async call:
 		- new thread created in Runtime and pushed to top of Runtime.active stack
 		- pushes something onto Frame.eval_stack that can be used to access return value (future/promise/cbcb)
 		- running thread unchanged and continues execution immediately after call
	- when o() called:
 		- callback that powers return value called with argument provided

 	Parallel Operation:
 	- new runtime with parallel call in RT.running
 	-
	- similar to async...


*/


#endif //SCL_VM_HPP
