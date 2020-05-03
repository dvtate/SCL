//
// Created by tate on 02-05-20.
//

#include <istream>
#include <fstream>

#include "compile.hpp"
#include "../parse/parse.hpp"



inline AST get_ast(std::string fname) {
	std::ifstream file = std::ifstream(fname);
	return parse(tokenize_stream(file));
}


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
	const size_t lit_num = this->compiler->literals.size();
	this->compiler->literals.emplace_back(v);

	const size_t new_pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_LIT, 0))
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


//
Program::Program(std::string fname) {
	AST main = get_ast(fname);
	std::vector<SemanticError> errs = process_tree(main, fname);
	ParsedMacro(main, fname, std::vector<ParsedMacro*>{}, this);
}

// emplace a parsed literal into literals header
// return index of parsed literal
// if its already in header send index
std::size_t Program::add_lit(ParsedLiteral&& lit) {
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