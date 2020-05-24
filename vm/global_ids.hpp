//
// Created by tate on 23-05-20.
//

#ifndef DLANG_GLOBAL_IDS_HPP
#define DLANG_GLOBAL_IDS_HPP

#include <cinttypes>
#include "handle.hpp"

class Value;
Handle<Value> get_global_id(int64_t id);

#endif //DLANG_GLOBAL_IDS_HPP
