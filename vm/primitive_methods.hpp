//
// Created by tate on 19/05/2021.
//

#ifndef SCL_PRIMITIVE_METHODS_HPP
#define SCL_PRIMITIVE_METHODS_HPP

#include <string>
#include "value.hpp"

Value get_primitive_member(Frame& f, Value& v, const std::string& key);

#endif //SCL_PRIMITIVE_METHODS_HPP
