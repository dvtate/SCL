//
// Created by tate on 02-05-20.
//

#ifndef SCL_COMPILE_HPP
#define SCL_COMPILE_HPP

#include <cinttypes>
#include <string>
#include <unordered_map>
#include <vector>
#include <list>
#include <forward_list>
#include <memory>

#include "semantics/semantics.hpp"
#include "command.hpp"

/*
 * main program file is implicitly converted into a macro and all the global identifiers are loaded
 *
 */

class ParsedMacro;

// all identifiers are replaced with numbers so we don't have to do hashes or put strings in bytecode
class MutilatedSymbol {
public:

	// unique numeric identifier for user-defined symbol
	// hash(int) is faster than hash(string)
	static int64_t _uid;
	int64_t id;
	std::string name;

	// Alias substitution
	std::shared_ptr<ParsedMacro> substitution;
	// TODO alias should use different type than ParsedMacro as we only want to have symbols bound and not change syntax meaning/context

	enum SymbolType {
		VARIABLE,
		ALIAS,
		CONSTANT,
		NO_REASSIGN
	} type {VARIABLE};

	MutilatedSymbol(): id(_uid++), substitution(nullptr) {}
	explicit MutilatedSymbol(const int64_t id, const SymbolType type = SymbolType::VARIABLE):
		id(id), substitution(nullptr), type(type) {}
	explicit MutilatedSymbol(std::string name, const SymbolType type = SymbolType::VARIABLE):
		id(_uid++), name(std::move(name)), substitution(nullptr), type(type) {}
	MutilatedSymbol(std::string name, const int64_t id, const SymbolType type = SymbolType::VARIABLE):
		id(id), name(std::move(name)), substitution(nullptr), type(type) {}
	MutilatedSymbol(std::string name, const int64_t id, ParsedMacro* substitution, const SymbolType type = SymbolType::VARIABLE):
		id(id), name(std::move(name)), substitution(substitution), type(type) {}
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

	// Lexical parents
	std::vector<ParsedMacro*> parents;

	// where did different commands originate in source program
	std::vector<std::pair<std::size_t, unsigned long long>> relocation;

	//
	std::vector<SemanticError> errors;

	Program* compiler;

	ParsedMacro(AST& tree, std::string file_name,
			std::vector<ParsedMacro*> parents, Program* prog,
			std::unordered_map<std::string, MutilatedSymbol> locals = {});
	ParsedMacro(const ParsedMacro& other) = default;
	ParsedMacro() = default;

	MutilatedSymbol find_id(const std::string& name);
	int64_t declare_id(const std::string& id_name);

	// Context that needs to be passed to children
	struct context_arg {
		bool lhs_eq : 1;
	};

	//
	struct context_ret {
		bool immutable : 1;
	};

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
	void read_assignment(AST&);
	void read_dot_op(AST&);
	void read_obj_lit(AST&);

	// Returns a new parsed macro containing compiled contents of arg
	ParsedMacro* compile_expr(AST&);
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
	void load_file(const char* file_name);

	inline explicit Program(const char* file_name) { load_file(file_name); }
	Program() = default;
};


#endif //SCL_COMPILE_HPP
