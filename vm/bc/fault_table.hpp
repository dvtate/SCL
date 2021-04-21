//
// Created by tate on 10/04/2021.
//

#ifndef SCL_FAULT_TABLE_HPP
#define SCL_FAULT_TABLE_HPP

#include <cinttypes>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <istream>

struct FaultTable {
	std::unordered_map<uint64_t, std::pair<std::shared_ptr<std::string>, uint64_t>> relocations;
	std::unordered_map<uint64_t, std::string> source_ids;
	std::unordered_map<uint64_t, std::string> invoke_lhs;

	static FaultTable* read(std::istream& is);
};

#endif //SCL_FAULT_TABLE_HPP
