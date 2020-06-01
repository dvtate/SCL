//
// Created by tate on 17-05-20.
//

#ifndef DLANG_VM_HPP
#define DLANG_VM_HPP

#include <iostream>

#include <thread>
#include <mutex>

#include <vector>
#include <queue>
#include <cstdint>
#include <list>

#include "closure.hpp"
#include "bc/exec_bc_instr.hpp"
#include "literal.hpp"
#include "value.hpp"


class Frame;
class Runtime;
class VM;
class Value;

//using SyncCallStack = std::vector<std::shared_ptr<Frame>>;

// abstract type used for handling return values and such
class RTMessage {
public:
	virtual ~RTMessage(){};
	virtual void action(Runtime&) = 0;
};


class SyncCallStack : public std::vector<std::shared_ptr<Frame>> {
public:
	std::shared_ptr<SyncCallStack> creator;
};


class Frame {
public:

	Runtime* rt;

	// closure we're running
	Closure closure;

	// index of bytecode read head
	unsigned int pos;

	// values we're working with
	std::vector<Value> eval_stack;

	Frame(Runtime* rt, Closure body, unsigned int pos = 0, std::vector<Value> eval_stack = {}):
			rt(rt), closure(std::move(body)), pos(pos), eval_stack(std::move(eval_stack))
		{}

	// run a single bytecode instruction and return
	inline bool tick(){
		if (pos >= this->closure.body->size())
			return true;
		exec_bc_instr(*this, (*this->closure.body)[this->pos++]);
		return false;
	}

};

// different threads running on same process
class Runtime {
public:
	VM* vm;
	// currently running thread
	std::shared_ptr<SyncCallStack> running;

	// threads to execute [stack]
	std::vector<std::shared_ptr<SyncCallStack>> active;

	// threads that still have pending actions
	std::vector<std::shared_ptr<SyncCallStack>> undead;

	// thread safety as messages can come from different procs/ISR's
	std::mutex msg_queue_mtx;

	explicit Runtime(VM* vm): vm(vm) { }

	// pushes msg onto msg queue
	void recv_msg(RTMessage* msg) {
		std::lock_guard<std::mutex> m(this->msg_queue_mtx);
		this->_msg_queue.emplace_back(msg);
		std::cout <<"recv msg\n";
	}

	// clears msg queue and returns old contents
	// NOTE: expects caller to free() pointers
	std::vector<RTMessage*> clear_msg_queue() {
		std::vector<RTMessage*> cpy = {};
		std::lock_guard<std::mutex> m(this->msg_queue_mtx);
		std::swap(cpy, this->_msg_queue);
		return cpy;
	}


	//
	void freeze_running() {
		if (this->active.empty()) {
			this->running = nullptr;
			return;
		}
		this->running = this->active.back();
		this->active.pop_back();
	}

	//
	void freeze_active(const std::shared_ptr<SyncCallStack>& cs) {
		for (auto it = this->active.begin(); it < this->active.end(); it += 1)
			if (*it == cs) {
				this->active.erase(it);
				return;
			}
	}

	// removes call stack from undead tracker
	void kill(const std::shared_ptr<SyncCallStack>& cs){
		for (auto it = this->undead.begin(); it < this->undead.end(); it += 1)
			if (*it == cs) {
				this->undead.erase(it);
				return;
			}
	}
	void kill_running(){
		auto cs = this->running;
		this->freeze_running();
		this->kill(cs);
	}

	void run();

private:
	// mutex guards on r/w
	// NOTE: treated as queue because all msgs read at once in FIFO order
	std::vector<RTMessage*> _msg_queue;

};


class VM {
public:

	std::vector<Literal> literals;

	std::shared_ptr<Runtime> main_thread;
	std::list<std::shared_ptr<Runtime>> worker_threads;

	VM(std::vector<Literal> lit_header, std::vector<std::string> argv);

	void run();
};

/*
	Making calls:
	- on sync call, new Frame is pushed onto running.call_stack()
	- on async call, new thread is created and pushed to top of active stack
	- on parallel call, new Runtime is pushed onto VM.worker_threads()

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


#endif //DLANG_VM_HPP
