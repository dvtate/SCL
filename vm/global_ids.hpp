//
// Created by tate on 23-05-20.
//

#ifndef SCL_GLOBAL_IDS_HPP
#define SCL_GLOBAL_IDS_HPP

#include <cinttypes>


// Using these internally too
enum class GlobalId : int64_t {
	EMPTY = 0,
	PRINT = 1,
	INPUT = 2,
	IF = 3,
	STR = 4,
	NUM = 5,
	VARS = 6,
	ASYNC = 7,
	IMPORT = 8,
	SIZE = 9,
	COPY = 10,
	ERROR = 11,
	THROW = 12,
};

class Value;
const Value& get_global_id(int64_t id);
constexpr unsigned short global_ids_count = 12;

#endif //SCL_GLOBAL_IDS_HPP
