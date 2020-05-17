//
// Created by tate on 17-05-20.
//

#ifndef DLANG_LITERAL_HPP
#define DLANG_LITERAL_HPP

#include <cinttypes>
#include <variant>
#include <string>
#include <vector>

#include "closure.hpp"

class ClosureTemplate {
	std::vector<int64_t> capture_ids;
	std::vector<int64_t> decl_ids;
	char* body;


};

class Literal {
	std::variant<ClosureTemplate, std::string> v;

};


#endif //DLANG_LITERAL_HPP
