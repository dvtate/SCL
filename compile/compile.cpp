//
// Created by tate on 02-05-20.
//

#include <istream>
#include <fstream>

#include "compile.hpp"
#include "bytecode.hpp"
#include "../parse/parse.hpp"




// maybe later want first 10 symbol ids to be reserved
int64_t MutilatedSymbol::_uid = 10;

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

	// add command and update relocations
	const size_t new_pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_LIT, lit_num));
	this->relocation.emplace_back(
			std::pair<std::size_t, unsigned long long>{
				new_pos, tree.token.pos });
}

void ParsedMacro::read_decl(AST& tree) {
	if (tree.members.empty()) {
		this->errors.emplace_back(SemanticError(
				"empty declaration", tree.token.pos, this->file_name));
		return;
	}

	// support multiple declarations separated by commas
	if (tree.members[0].type == AST::NodeType::OPERATOR && tree.token.token == ",")
		tree.members = tree.members[0].members;

	for (auto& d : tree.members) {
		if (d.type == AST::NodeType::IDENTIFIER) {
			this->declare_id(d.token.token);
		} else if (d.type == AST::NodeType::OPERATOR && d.token.token == "=") {
			if (!d.members.empty() && d.members[0].type == AST::NodeType::IDENTIFIER) {
				this->declare_id(d.members[0].token.token);
				this->read_tree(d);
			} else {
				this->errors.emplace_back(SemanticError(
						"invalid assignment, expected identifier, got: " + (
								d.members.empty() ? d.type_name() : d.members[0].type_name()),
						d.token.pos, this->file_name));
			}
		} else {
			this->errors.emplace_back(SemanticError(
					"Invalid declaration, unexpected " + d.type_name(),
					d.token.pos, this->file_name));
		}
	}
}

void ParsedMacro::read_id(AST& tree) {
	const uint64_t id = this->find_id(tree.token.token);
	if (!id) {
		this->errors.emplace_back(SemanticError(
				"Identifier used before declaration",
				tree.token.pos,
				this->file_name));
		return;
	}

	const size_t n_pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_ID, (int64_t) id));
	this->relocation.emplace_back(
			std::pair<std::size_t, unsigned long long>{
				n_pos, tree.token.pos });

}

void ParsedMacro::read_operation(AST& t){

	// TODO: replace with actual operator ID's from VM
	std::unordered_map<std::string, uint16_t> op_ids {
		{ "!",	10 },
		{ "neg",	11 },
		{ "**",	12 },
		{ "*",	13 },
		{ "/",	14 },
		{ "%",	15 },
		{ "+", 	16 },
		{ "-",	17 },
		{ "<",	18 },
		{ ">",	19 },
		{ "==",	20 },
	};

	// check if it forms an expression or just lexical
	uint16_t op;
	try {
		op = op_ids.at(t.token.token);
	} catch (...) {
		op = 0;
	}
	if (!op) {
		this->errors.emplace_back(SemanticError(
				"Unexpected lexical operator: " + t.token.token,
				t.token.pos, this->file_name));
		return;
	}

	// compile arguments
	for (auto& arg : t.members)
		read_tree(arg);

	std::size_t lpos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::BUILTIN_OP, op));
	this->relocation.emplace_back(std::pair { lpos, t.token.pos });
}

void ParsedMacro::read_macro_invoke(AST& t) {
	if (t.members.size() < 2) {
		this->errors.emplace_back(SemanticError(
				"Invalid macro call", t.token.pos, this->file_name));
		return;
	}
	const auto id = find_id(t.members[0].token.token);
	if (!id) {
		this->errors.emplace_back(SemanticError(
				"macro call on undeclared identifier, " + t.members[0].token.token,
				t.token.pos,this->file_name));
		return;
	}

	for (auto m : t.members)
		this->read_tree(m);

	this->body.emplace_back(Command(Command::OPCode::MACRO_INVOKE));


}

void ParsedMacro::read_macro_lit(AST& tree) {
	// TODO: :/

}



void ParsedMacro::read_statements(AST& tree) {
	for (AST& statement : tree.members) {
		read_tree(statement);
		this->body.emplace_back(Command(Command::OPCode::CLEAR_STACK));
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
			return;
		case AST::NodeType::IDENTIFIER:
			read_id(tree);
			return;
		case AST::NodeType::OPERATION:
			read_operation(tree);
			return;

		default:
			this->errors.emplace_back(SemanticError(
					"Unsupported feature: " + tree.type_name(),
					tree.token.pos, this->file_name));
	}

}

//
ParsedMacro::ParsedMacro(AST &tree, std::string file_name, std::vector<ParsedMacro *> parents,
		Program* prog, std::unordered_map<std::string, MutilatedSymbol> locals):
	file_name(std::move(file_name)), parents(std::move(parents)), compiler(prog)
{
	declarations = std::move(locals);
	declarations["o"] = MutilatedSymbol("o");
	declarations["i"] = MutilatedSymbol("i");
}

uint64_t ParsedMacro::find_id(const std::string& name) {
	auto it = this->declarations.find(name);
	if (it != this->declarations.end())
		return it->second.id;

	// check previous scopes
	for (ParsedMacro* parent : parents) {
		it = parent->declarations.find(name);
		if (it != parent->declarations.end())
			return it->second.id;
	}

	// not found
	return 0;
}

void Program::load_macro(ParsedMacro& macro) {

	// copy identifiers
	for (auto& p : macro.declarations)
		this->identifiers.emplace_back(p.second);

	// copy translated positions to go in fault table
	// FIXME: move this step to compilation as we don't know size of other literals
//	auto it = this->translated_positions.find(macro.file_name);
//	if (it == this->translated_positions.end())
//		this->translated_positions[macro.file_name] = macro.relocation;
//	else
//		this->translated_positions[macro.file_name].insert(
//			this->translated_positions[macro.file_name].end(),
//			macro.relocation.begin(), macro.relocation.end());

	this->empl_lit(ParsedLiteral(macro));
}

//
Program::Program(std::string fname) {
	load_file(fname);
}



void Program::load_file(const std::string& fname) {
	// parse main file
	std::ifstream file = std::ifstream(fname);
	AST main = parse(tokenize_stream(file));

	// semantic analysis
	std::vector<SemanticError> errs = process_tree(main, fname);

	// implicit main macro
	ParsedMacro entry(main, fname,
			std::vector<ParsedMacro*>{}, this,
			std::unordered_map<std::string, MutilatedSymbol>{
		{ "print", MutilatedSymbol("print") },
		{ "input", MutilatedSymbol("input") }
	});

	// dfs children of entry
	// there has to be a better solution than this,,,,
	std::vector<ParsedMacro> macros_processed; // these are going into literals header
	std::vector<ParsedMacro> macros_to_proc { entry.children }; // these are going to be processed

	while (!macros_to_proc.empty()) {
		this->load_macro(macros_to_proc.back());
		macros_processed.emplace_back(macros_to_proc.back());
		macros_to_proc.pop_back();
		macros_to_proc.insert(macros_to_proc.end(),
				macros_processed.back().children.begin(),
				macros_processed.back().children.end());

	};

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

std::vector<SemanticError> Program::compile(std::vector<Command>& ret) {
	std::vector<SemanticError> errs;

	// compile literals header
	for (auto& lit : this->literals) {
		switch (lit.type) {
			case ParsedLiteral::LitType::STRING:
				ret.emplace_back(Command(
						Command::OPCode::START_LIT_STRING,
						std::get<std::string>(lit.v)));
				break; // switch

			case ParsedLiteral::LitType::JSON:
				ret.emplace_back(Command(
						Command::OPCode::START_LIT_JSON,
						std::get<std::string>(lit.v)));
				break; // switch

			case ParsedLiteral::LitType::MACRO: {
				const auto macro = std::get<ParsedMacro>(lit.v);

				// offset for position updates
				const std::size_t start_pos = ret.size();
				ret.emplace_back(Command(Command::OPCode::START_LIT_MACRO));
				ret.insert(ret.end(), macro.body.begin(), macro.body.end());
				ret.emplace_back(Command(Command::OPCode::END_LIT_MACRO));

				// include macro specific relocations in global relocations table for fault table
				auto &tp = this->translated_positions[macro.file_name];
				for (auto &p : macro.relocation)
					tp.emplace_back(std::pair{p.first + start_pos, p.second});

				break; // switch
			}
		}
	}

	ret.emplace_back(Command(Command::OPCode::END_LIT_SECTION));


	// compile fault table
	// this section of binary is so that we can generate meaningful runtime error messages

	for (auto& sym : this->identifiers) {
		ret.emplace_back(Command(Command::OPCode::ID_NAME, sym.name));
		ret.emplace_back(Command(Command::OPCode::ID_ID, (int64_t) sym.id));
	}

	for (auto& f : this->translated_positions) {
		ret.emplace_back(Command(Command::OPCode::FILE_NAME, f.first));
		for (auto& p : f.second) {
			ret.emplace_back(Command(Command::OPCode::DEST_POS, (int64_t) p.first));
			ret.emplace_back(Command(Command::OPCode::SRC_POS, (int64_t) p.second));
		}
	}

	return errs;
}