//
// Created by tate on 02-05-20.
//

#ifndef DLANG_COMPILE_HPP
#define DLANG_COMPILE_HPP

#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <forward_list>

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
	static int64_t _uid;
	int64_t id;
	std::string name;

	MutilatedSymbol(): id(_uid++) {}
	explicit MutilatedSymbol(std::string uname): id(_uid++), name(std::move(uname)) {}
	MutilatedSymbol(const std::string name, int64_t id): id(id), name(std::move(name)) {}
	MutilatedSymbol(const MutilatedSymbol& cpy) = default;
};

class Program;

// everything is a macro
// bulk of compilation effort is done here as all user-level code is inside macros
class ParsedMacro {
public:
	// identifiers declared in macro scope
	std::unordered_map<std::string, MutilatedSymbol> declarations;

	// TODO: input/output types

	// code in body
	std::vector<Command> body;

	// file name of definition
	std::string file_name;

	std::vector<ParsedMacro*> parents;

	// where did different commands originate in source program
	std::vector<std::pair<std::size_t, unsigned long long>> relocation;

	std::vector<SemanticError> errors;

	Program* compiler;

	ParsedMacro(AST& tree, std::string file_name,
			std::vector<ParsedMacro*> parents, Program* prog,
			std::unordered_map<std::string, MutilatedSymbol> locals = {});
	ParsedMacro(const ParsedMacro& other) = default;

	int64_t find_id(const std::string& name);
	int64_t declare_id(const std::string& id_name);

	// translating different branch types into bytecode and populating internal structures
	void read_tree(AST&); // main entry
	void read_statements(AST&);
	void read_num_lit(AST&);
	void read_string_lit(AST&);
	void read_decl(AST&);
	void read_id(AST&);
	void read_operation(AST&);
	void read_macro_lit(AST&);
	void read_list_lit(AST&);
	void read_macro_invoke(AST&);
	void read_index_op(AST&);
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

		// make compiler stop complain
		return false;
	}

};

// combining different systems and converting different data to desired formats
class Program {
public:
	// literals.back() == main entry point
	std::vector<ParsedLiteral> literals;

	// these are used for making the fault table
	std::vector<MutilatedSymbol> identifiers;
	std::unordered_map<std::string, std::vector<std::pair<std::size_t, unsigned long long>>> translated_positions;

	std::vector<SemanticError> compile(std::vector<Command>& ret);

	// emplace a parsed literal into literals header
	// return literal index
	int64_t empl_lit(ParsedLiteral&& lit);

	// used to populate internal structures
	int64_t load_macro(ParsedMacro& macro);
	void load_file(const std::string& fname);

	inline explicit Program(const std::string& fname) { load_file(fname); }
	Program() = default;
};


#endif //DLANG_COMPILE_HPP
