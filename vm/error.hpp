//
// Created by tate on 20/04/2021.
//

#ifndef SCL_ERROR_HPP
#define SCL_ERROR_HPP

#include <string>
#include <vector>
#include <utility>
#include <memory>

#include "bc/bc.hpp"

#include "value_types.hpp"
#include "value.hpp"

class Frame;

// catch()
class CatchFn : public NativeFunction {
	std::shared_ptr<Frame> frame;
public:
	CatchFn(Frame& f);
	void operator()(Frame& f) override;
	void mark() override;
};


class Frame;
class SyncCallStack;

class ErrorTrace {
public:
	// array of pairs of instruction positions and macro body pointers so that we can reconstruct
	// absolute position later
	std::vector<std::pair <uint64_t, std::vector < BCInstr>*>> trace;

	ErrorTrace(SyncCallStack& cs);

	//
	std::string depict(Frame& f, const std::string& name, const std::string& message);

	// Add another call stack to the error object
	void extend(SyncCallStack& cs);
};

class ErrorTraceStrFn : public NativeFunction {
public:
	std::shared_ptr <Frame> frame;
	ErrorTrace trace;
	Value self;

	ErrorTraceStrFn(Frame &f, Value self);
	void operator()(Frame& f) override;
	void mark() override;
};


//
Value gen_error_object(const std::string name, const std::string message, Frame& f);

#endif //SCL_ERROR_HPP
