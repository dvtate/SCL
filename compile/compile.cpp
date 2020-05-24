//
// Created by tate on 02-05-20.
//

#include <istream>
#include <fstream>

#include "compile.hpp"
#include "bytecode.hpp"
#include "../parse/parse.hpp"


#ifdef DLANG_DEBUG
	#define DLANG_DEBUG_MSG(m) std::cout <<m;
#else
	#define DLANG_DEBUG_MSG(m)
#endif

const static std::unordered_map<std::string, Command> keyword_values = {
		{ "empty", Command(Command::OPCode::KW_VAL, (uint16_t) 0) },
		{ "true",  Command(Command::OPCode::KW_VAL, (uint16_t) 1) },
		{ "false", Command(Command::OPCode::KW_VAL, (uint16_t) 2) },
		{ "print", Command(Command::OPCode::KW_VAL, (uint16_t) 3) },
		{ "input", Command(Command::OPCode::KW_VAL, (uint16_t) 4) },
};

const static std::unordered_map<std::string, int64_t> global_ids {
	{ "print", 1 },
};

// maybe later want first 10 symbol ids to be reserved
int64_t MutilatedSymbol::_uid = 10;

// ParsedMacro::read_* : recursively convert the AST of the macro into a list of commands and populate structures

void ParsedMacro::read_num_lit(AST& tree) {
	DLANG_DEBUG_MSG("read_num_lit\n");
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
	DLANG_DEBUG_MSG("read_string_lit\n");

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
	DLANG_DEBUG_MSG("read_decl\n");
	if (tree.members.empty()) {
		this->errors.emplace_back(SemanticError(
				"empty declaration", tree.token.pos, this->file_name));
		return;
	}

	// support multiple declarations separated by commas
	if (tree.members[0].type == AST::NodeType::OPERATION && tree.members[0].token.token == ",")
		tree.members = tree.members[0].members;

	for (auto& d : tree.members) {
		DLANG_DEBUG_MSG("read_decl:" <<debug_AST(d) <<std::endl);
		if (d.type == AST::NodeType::IDENTIFIER) {
			if (keyword_values.find(d.token.token) != keyword_values.end()) {
				this->errors.emplace_back(SemanticError(
					"Redefinition of global keyword "+ d.token.token,
					d.token.pos, this->file_name, true
				));
			}
			this->declare_id(d.token.token);

		} else if (d.type == AST::NodeType::OPERATION && d.token.token == "=") {
			if (!d.members.empty() && d.members[0].type == AST::NodeType::IDENTIFIER) {
				if (keyword_values.find(d.members[0].token.token) != keyword_values.end()) {
					this->errors.emplace_back(SemanticError(
							"Redefinition of global keyword "+ d.members[0].token.token,
							d.members[0].token.pos, this->file_name, true
					));
				}
				auto idid = this->declare_id(d.members[0].token.token);
				this->body.emplace_back(Command(Command::OPCode::DECL_ID, idid));
				this->read_tree(d);
				DLANG_DEBUG_MSG("read_decl: "<<d.members[0].token.token <<" = " <<idid <<std::endl);
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
	DLANG_DEBUG_MSG("read_id\n");

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
	DLANG_DEBUG_MSG("read_operation\n");
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
		{ "=",	21 },
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
	DLANG_DEBUG_MSG("read_macro_invoke\n");

	if (t.members.size() < 1) {
		this->errors.emplace_back(SemanticError(
				"Invalid macro call", t.token.pos, this->file_name));
		return;
	}

	// load arg
	if (t.members.size() == 2)
		read_tree(t.members.back());
	else // no arg
		this->body.emplace_back(Command(Command::OPCode::VAL_EMPTY));

	// load macro
	read_tree(t.members[0]);

	// TODO: typecheck

	this->body.emplace_back(Command(Command::OPCode::INVOKE));


}

void ParsedMacro::read_macro_lit(AST& tree) {
	// TODO: :/

}



void ParsedMacro::read_statements(AST& tree) {
	DLANG_DEBUG_MSG("read_statements\n");
	for (AST& statement : tree.members) {
		read_tree(statement);
		this->body.emplace_back(Command(Command::OPCode::CLEAR_STACK));
	}
}

void ParsedMacro::read_tree(AST& tree) {
	DLANG_DEBUG_MSG("read_tree: " <<tree.type_name());
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

		case AST::NodeType::MACRO_INVOKE:
			read_macro_invoke(tree);
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
	MutilatedSymbol i("i"), o("o");
	declarations["o"] = o;
	declarations["i"] = i;
	this->body.emplace_back(Command(Command::OPCode::DECL_ID, i.id));
	this->body.emplace_back(Command(Command::OPCode::DECL_ID, o.id));

}

int64_t ParsedMacro::find_id(const std::string& name) {
	// TODO: handle others
	// global id
	if (name == "print")
		return 1;
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

int64_t ParsedMacro::declare_id(const std::string& id_name) {
	auto occ = this->declarations.find(id_name);
	if (occ == this->declarations.end()) {
		MutilatedSymbol&& ms = MutilatedSymbol(id_name);
		const int64_t ret = ms.id;
		declarations[id_name] = ms;
		return ret;
	}
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
			// NOTE: these are technically global... just a hack
			std::unordered_map<std::string, MutilatedSymbol>{
		{ "print", MutilatedSymbol("print", 0) },
		{ "input", MutilatedSymbol("input", 1) }
	});
	entry.read_tree(main);

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

				// recombine errors
				errs.insert(errs.end(), macro.errors.begin(), macro.errors.end());

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
			ret.emplace_back(Command(Command::OPCode::SRC_POS,  (int64_t) p.second));
		}
	}

	return errs;
}