//
// Created by tate on 02-05-20.
//

#include <istream>
#include <fstream>

#include "compile.hpp"
#include "../parse/parse.hpp"




// maybe later want first 10 symbol ids to be reserved
uint64_t MutilatedSymbol::_uid = 10;

// read_* : recursively convert the AST of the macro into a list of commands and populate structures

void ParsedMacro::read_num_lit(AST& tree) {

	try { // try to parse int first
		// sigh...
		int64_t v;
		if (sizeof(long) == sizeof(int64_t)) {
			v = std::stol(tree.token.token);
		} else if (sizeof(long long) == sizeof(int64_t)) {
			v = std::stoll(tree.token.token);
		} else {
			v = std::stoi(tree.token.token);
		}
		this->body.emplace_back(Command(Command::OPCode::I64_LIT, v));
	} catch (...) {
		try {
			// not an int, parse double
			const double v = std::stod(tree.token.token);
			this->body.emplace_back(Command(Command::OPCode::F64_LIT, v));
		} catch (...) {
			this->errors.emplace_back(SemanticError(
					"Invalid number literal" + tree.token.token,
					tree.token.pos,
					this->file_name));
		}
	}

	// source location mapping
	this->relocation.emplace_back(std::pair<std::size_t, unsigned long long>{
		this->body.size() - 1,
		tree.token.pos
	});


	//Command c(Command::OPCode::I64_LIT);
}

void ParsedMacro::read_string_lit(AST& tree) {

	// get string text
	std::string v = tree.token.token;

	// put string into literals header
	const int64_t lit_num = this->compiler->empl_lit(
			ParsedLiteral(ParsedLiteral::LitType::STRING, v));
	const size_t new_pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_LIT, lit_num));
	this->relocation.emplace_back(
			std::pair<std::size_t, unsigned long long>{
				new_pos, tree.token.pos });
}

void ParsedMacro::read_decl(AST& tree) {
	// assuming it starts with let...
}

void ParsedMacro::read_statements(AST& tree) {
	for (AST& statement : tree.members) {
		read_tree(statement);
	}
}

void ParsedMacro::read_tree(AST& tree) {
	switch (tree.type) {
//		case AST::NodeType::OPERATOR:
//
//			return;
		case AST::NodeType::STATEMENTS:
			read_statements(tree);
			return;
		case AST::NodeType::STR_LITERAL:
			read_string_lit(tree);
			return;
		case AST::NodeType::NUM_LITERAL:
			read_num_lit(tree);
			return;
		case AST::NodeType::DECLARATION:
			read_decl(tree);

	}

}

//
ParsedMacro::ParsedMacro(AST &tree, std::string file_name, std::vector<ParsedMacro *> parents, Program* prog):
		decl_id({
						{"o", MutilatedSymbol("o")},
						{"i", MutilatedSymbol("i")}
				}), file_name(std::move(file_name)), parents(std::move(parents)), compiler(prog)
{

}

uint64_t ParsedMacro::find_id(const std::string& name) {
	auto it = this->decl_id.find(name);
	if (it != this->decl_id.end())
		return it->second.id;

	// check previous scopes
	for (ParsedMacro* parent : parents) {
		it = parent->decl_id.find(name);
		if (it != parent->decl_id.end())
			return it->second.id;
	}

	// not found
	return 0;
}

void Program::load_macro(ParsedMacro& macro) {

	// copy identifiers
	for (const auto& p : macro.decl_id)
		this->identifiers.emplace_back(p.second);

	// copy translated positions to go in fault table
	auto it = this->translated_positions.find(macro.file_name);
	if (it == this->translated_positions.end())
		this->translated_positions[macro.file_name] = macro.relocation;
	else
		this->translated_positions[macro.file_name].insert(
			this->translated_positions[macro.file_name].end(),
			macro.relocation.begin(), macro.relocation.end());

	this->empl_lit(ParsedLiteral(macro));
}

//
Program::Program(std::string fname) {
	// parse main file
	std::ifstream file = std::ifstream(fname);
	AST main = parse(tokenize_stream(file));

	// semantic analysis
	std::vector<SemanticError> errs = process_tree(main, fname);

	// implicit main macro
	ParsedMacro entry(main, fname, std::vector<ParsedMacro*>{}, this);

	// dfs children of entry
	std::vector<ParsedMacro> macros_processed; // these are going into literals header
	std::vector<ParsedMacro> macros_to_proc { entry.children }; // these are going to be processed

	do {
		this->load_macro(macros_to_proc.back());
		macros_processed.emplace_back(macros_to_proc.back());
		macros_to_proc.pop_back();
		macros_to_proc.insert(macros_to_proc.end(),
				macros_processed.back().children.begin(),
				macros_processed.back().children.end());

	} while (!macros_to_proc.empty());

	// literals.back() == main()
	this->load_macro(entry);
}

// emplace a parsed literal into literals header
// return index of parsed literal
// if its already in header send index
std::size_t Program::empl_lit(ParsedLiteral&& lit) {
	auto it = std::find(
			this->literals.begin(),
			this->literals.end(),
			lit);

	if (it == this->literals.end()) {
		std::size_t ret = this->literals.size();
		this->literals.emplace_back(lit);
		return ret;

	} else {
		return std::distance(this->literals.begin(), it);
	}
}