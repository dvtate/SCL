//
// Created by tate on 23-05-20.
//

#ifndef SCL_GLOBAL_IDS_HPP
#define SCL_GLOBAL_IDS_HPP

#include <cinttypes>

class Value;
const Value& get_global_id(int64_t id);
constexpr unsigned short global_ids_count = 12;

#endif //SCL_GLOBAL_IDS_HPP
