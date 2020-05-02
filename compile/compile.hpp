//
// Created by tate on 02-05-20.
//

#ifndef DLANG_COMPILE_HPP
#define DLANG_COMPILE_HPP

#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>

#include "semantics/process_tree.hpp"
#include "command.hpp"

class MutilatedSymbol {
public:

	// unique numeric identifier for user-defined symbol
	// hash(int) is faster than hash(string)
	static uint64_t _uid; // {0}
	uint64_t id;

	MutilatedSymbol(): id(_uid++) {}
	MutilatedSymbol(const MutilatedSymbol& cpy) = default;

};

class ParsedMacro {
public:
	// identifiers declared in macro scope
	std::unordered_map<std::string, MutilatedSymbol> decl_id;

	//
	std::vector<uint64_t> used_ids;

	// TODO: input/output types
	std::vector<Command> body;

	ParsedMacro(std::vector<Command> body);
	ParsedMacro(AST& t);
};

class ParsedLiteral {
public:
	std::variant<ParsedMacro, std::string> v;
	enum LitType {
		STRING, MACRO, JSON
	} type;

	bool operator==(const Literal& other) {
		if (this->type != other.type)
			return false;
		if (this->type == LitType::STRING || this->type == LitType::JSON)
			return std::get<std::string>(this->v) == std::get<std::string>(other.v);

		// TODO: return true if macros are compatible... (reduce number of simple, context-free macro literals)
		if (this->type == LitType::MACRO)
			return false;
	}
};


class

std::vector<Command> translate_tree(AST& t);

#endif //DLANG_COMPILE_HPP
