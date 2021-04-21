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

//
Value gen_error_object(const std::string name, const std::string message, Frame& f);

#endif //SCL_ERROR_HPP
