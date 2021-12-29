//
// Created by tate on 03/07/2021.
//

#ifndef SCL_TIMEPOINT_HPP
#define SCL_TIMEPOINT_HPP

#include <chrono>
#include <ctime>

#include "../../vm/vm.hpp"
#include "../../vm/error.hpp"

// TODO see timepoint.spec.txt
// std::chrono is utter garbage and ctime isn't much better

class TimePointFn : public NativeFunction {
	static double get_unix_ms() {
		auto tp = std::chrono::utc_clock::now();
		return tp.time_since_epoch().count();
	}

public:
	void operator()(Frame& f) override {
		auto arg = f.eval_stack.back();
		std::timeval tv;
		std::timespec ts;
		std::timespec_get(&ts, TIME_UTC)

		auto* ret = f.gc_make<ValueTypes::obj_t>({
			 { "ts", Value(get_unix_ms()) },
			 { "tz", Value("UTC") },
		});

		f.eval_stack.back() = Value(ret);
	}
	void mark() override {}
};

#endif //SCL_TIMEPOINT_HPP
