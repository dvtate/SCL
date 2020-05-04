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

/*
 * main program file is implicitly converted into a macro and all the global identifiers are loaded
 *
 */

// all identifiers are replaced with numbers so we don't have to do hashes or put strings in bytecode
class MutilatedSymbol {
public:

	// unique numeric identifier for user-defined symbol
	// hash(int) is faster than hash(string)
	static uint64_t _uid; // {0}
	uint64_t id;
	std::string name;

	MutilatedSymbol(): id(_uid++) {}
	explicit MutilatedSymbol(const std::string uname): id(_uid++), name(std::move(uname)) {}
	MutilatedSymbol(const MutilatedSymbol& cpy) = default;

};

class Program;

// everything is a macro

class ParsedMacro {
public:
	// identifiers declared in macro scope
	std::unordered_map<std::string, MutilatedSymbol> decl_id;

	// TODO: input/output types

	// code in body
	std::vector<Command> body;

	//
	std::vector<ParsedMacro> children;
	std::vector<ParsedMacro*> parents;

	// file name of definition
	std::string file_name;

	// where did different commands originate in source program
	std::vector<std::pair<std::size_t, unsigned long long>> relocation;

	std::vector<SemanticError> errors;

	Program* compiler;

	ParsedMacro(AST& tree, std::string file_name,
			std::vector<ParsedMacro*> parents, Program* prog);
	ParsedMacro(const ParsedMacro& other) = default;

	inline uint64_t declare_id(const std::string& id_name) {
		MutilatedSymbol&& ms = MutilatedSymbol(id_name);
		const uint64_t ret = ms.id;
		decl_id[id_name] = ms;
		return ret;
	}

	// translating different branch types into bytecode and populating internal structures
	void read_tree(AST&);
	void read_statements(AST&);
	void read_num_lit(AST&);
	void read_string_lit(AST&);
	void read_decl(AST&);

	uint64_t find_id(const std::string& name);
};

class ParsedLiteral {
public:
	std::variant<ParsedMacro, std::string> v;
	enum LitType {
		STRING, MACRO, JSON
	} type;

	ParsedLiteral(LitType type, ParsedMacro macro):
		v(std::move(macro)), type(type) {}
	explicit ParsedLiteral(ParsedMacro macro):
		v(std::move(macro)), type(LitType::MACRO) {}
	ParsedLiteral(LitType type, std::string s):
		v(std::move(s)), type(type) {}

	bool operator==(const ParsedLiteral& other) {
		if (this->type != other.type)
			return false;
		if (this->type == LitType::STRING || this->type == LitType::JSON)
			return std::get<std::string>(this->v) == std::get<std::string>(other.v);

		// TODO: return true if macros are compatible... (reduce number of simple, context-free macros stored in literal header)
		if (this->type == LitType::MACRO)
			return false;
	}
};

// handles compilation
class Program {
public:
	// literals.back() == main entry point
	std::vector<ParsedLiteral> literals;

	// these are used for making the fault table
	std::vector<MutilatedSymbol> identifiers;
	std::unordered_map<std::string, std::vector<std::pair<std::size_t, unsigned long long>>> translated_positions;


	Program(std::string fname);

	std::vector<Command> compile();

	// emplace a parsed literal into literals header
	// return literal index
	std::size_t empl_lit(ParsedLiteral&& lit);

	void load_macro(ParsedMacro& macro);
};

// std::vector<Command> translate_tree(AST& t);

#endif //DLANG_COMPILE_HPP
