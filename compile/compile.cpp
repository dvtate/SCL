//
// Created by tate on 02-05-20.
//

#include <istream>
#include <fstream>
#include <unordered_map>

#include "../debug.hpp"
#include "compile.hpp"
#include "bytecode.hpp"
#include "../parse/parse.hpp"

const static std::unordered_map<std::string, Command> keyword_values = {
		{ "empty", Command(Command::OPCode::KW_VAL, (uint16_t) 0) },
		{ "true",  Command(Command::OPCode::KW_VAL, (uint16_t) 1) },
		{ "false", Command(Command::OPCode::KW_VAL, (uint16_t) 2) },
		{ "print", Command(Command::OPCode::KW_VAL, (uint16_t) 3) },
		{ "input", Command(Command::OPCode::KW_VAL, (uint16_t) 4) },
};


// First few symbol ids are reserved
int64_t MutilatedSymbol::_uid = 20;

// ParsedMacro::read_* : recursively convert the AST of the macro into a list of commands and populate structures

void ParsedMacro::read_num_lit(AST& tree) {
	DLANG_DEBUG_MSG("read_num_lit\n");
	try { // try to parse int first
		// sigh...
		int64_t v;

		// TODO replace this with <climits> LONG_BITS INT_BITS ... etc.
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

void ParsedMacro::read_dot_op(AST& tree) {
	DLANG_DEBUG_MSG("read_dot_op");

	auto& obj = tree.members[0];
	auto& mem = tree.members[1];

	this->read_tree(obj);

	// parser allows things like `obj.[1 + 2]` or `obj.(abc[1+2] + Str(66))`
	// but for now at least we only want statically
	if (mem.type != AST::NodeType::IDENTIFIER) {
		this->errors.emplace_back(SemanticError(
				"Unsupported expression on right hand side of dot operator (.): " + mem.type_name(),
				mem.token.pos, this->file_name));

		// Scan subtrees to see if there are other errors
		this->read_tree(mem);
		return;
	}

	// add member name to lit header
	const std::string req = mem.token.token;
	const auto lit_num = this->compiler->empl_lit(
			ParsedLiteral(ParsedLiteral::LitType::STRING, req));

	// add
	const auto pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_MEM_L, lit_num));
	this->relocation.emplace_back(
			std::pair<std::size_t, unsigned long long> {
				pos, tree.token.pos});
}

void ParsedMacro::read_index_op(AST& tree){
	// put args on stack
	for (auto& m : tree.members)
		this->read_tree(m);
	this->body.emplace_back(Command(Command::OPCode::USE_INDEX));
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
			const auto idid = this->declare_id(d.token.token);
			this->body.emplace_back(Command(Command::OPCode::DECL_ID, idid));
		} else if (d.type == AST::NodeType::OPERATION && d.token.token == "=") {
			if (!d.members.empty() && d.members[0].type == AST::NodeType::IDENTIFIER) {
				if (keyword_values.find(d.members[0].token.token) != keyword_values.end()) {
					this->errors.emplace_back(SemanticError(
							"Redefinition of global keyword "+ d.members[0].token.token,
							d.members[0].token.pos, this->file_name, true
					));
				}
				const auto idid = this->declare_id(d.members[0].token.token);
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

	if (tree.token.token == "true") {
		this->body.emplace_back(Command::OPCode::VAL_TRUE);
		return;
	} if (tree.token.token == "false") {
		this->body.emplace_back(Command::OPCode::VAL_FALSE);
		return;
	}  if (tree.token.token == "empty") {
		this->body.emplace_back(Command::OPCode::VAL_EMPTY);
		return;
	}

	const MutilatedSymbol id = this->find_id(tree.token.token);
	if (id.id < 0) {
		this->errors.emplace_back(SemanticError(
				"Identifier used before declaration",
				tree.token.pos,
				this->file_name));
		return;
	}
	const size_t n_pos = this->body.size();
	this->body.emplace_back(Command(Command::OPCode::USE_ID, (int64_t) id.id));
	this->relocation.emplace_back(
			std::pair<std::size_t, unsigned long long>{
				n_pos, tree.token.pos });

}

//
void ParsedMacro::read_assignment(AST& t) {
	DLANG_DEBUG_MSG("read_assignment\n");

	// a = 123
	if (t.members[0].type == AST::NodeType::IDENTIFIER) {
		auto sym = this->find_id(t.members[0].token.token);
		if (sym.id < 0) {
			this->errors.emplace_back(SemanticError(
					"Left-hand side of equals not in scope", t.token.pos, this->file_name));
			return;
		}

		if (sym.type == MutilatedSymbol::SymbolType::CONSTANT) {
			this->errors.emplace_back(SemanticError(
					std::string("Cannot assign to constant `") + t.token.token + "`",
					t.token.pos, this->file_name));

		}
		if (sym.type == MutilatedSymbol::SymbolType::NO_REASSIGN) {
			this->errors.emplace_back(SemanticError(
					std::string("Left hand side of equals was marked as not assignable: `") + t.token.token + "`",
					t.token.pos, this->file_name));
		}

		// If lhs is an alias insert the compiled code
		// TODO alias should use different type than ParsedMacro
		if (sym.type == MutilatedSymbol::SymbolType::ALIAS) {
			read_tree(t.members[0]);
			read_tree(t.members[1]);
			// NOTE:
			this->body.emplace_back(Command(Command::OPCode::BUILTIN_OP, (uint16_t) 1));
		} else if (sym.type == MutilatedSymbol::SymbolType::VARIABLE) {
			// Set id
			read_tree(t.members[1]);
			this->body.emplace_back(Command(Command::OPCode::SET_ID, (int64_t) sym.id));
		}

		return;
	} else if (t.members[0].type == AST::NodeType::INDEX) {
		// list[index] = 5;  => <list> <index> <value> SET_INDEX

		// put args on stack
		for (auto &m : t.members[0].members)
			this->read_tree(m);
		this->read_tree(t.members[1]);
		// args: list index value
		this->body.emplace_back(Command(Command::OPCode::SET_INDEX));

	} else if (t.members[0].type == AST::NodeType::OPERATION && t.members[0].token.token == ".") {
		// obj.name = value; => <value> <obj> SET_MEM_L(litnum(name))

		// read value ("steve")
		this->read_tree(t.members[1]);

		// read member-request (USE_MEM_L) and convert to set_mem_l, other properties should be same
		this->read_tree(t.members[0]);
		this->body.back().instr = Command::OPCode::SET_MEM_L;

	} else {
		// Could be typerror
		this->errors.emplace_back(SemanticError(
				"Left hand size of equals is an unsupported expression type that must evaluate to a reference",
				t.token.pos, this->file_name, true));

		// compile arguments
		for (auto& arg : t.members)
			read_tree(arg);

		// Equals operator
		this->body.emplace_back(Command(Command::OPCode::BUILTIN_OP, (uint16_t) 1));
	}

	std::size_t lpos = this->body.size();
	this->relocation.emplace_back(std::pair { lpos, t.token.pos });
}

// operators defined only for convenience
void ParsedMacro::read_operation(AST& t){
	DLANG_DEBUG_MSG("read_operation\n");
	// TODO: replace with actual operator ID's from VM
	std::unordered_map<std::string, uint16_t> op_ids {
			{ "+",		0 },
			{ "=",		1 },
			{ "==",		2 },
			{ "===",	3 },
			{ "<",		4 },
			{ ">",		5 },
			{ "<=",		6 },
			{ ">=",		7 },
			{ "!",		8 },
			{ "-",		9 },
	};

	// check if it forms an expression or just lexical
	int32_t op;
	try {
		op = op_ids.at(t.token.token);
	} catch (...) {
		op = -1;
	}

	if (op < 0) {
		this->errors.emplace_back(SemanticError(
				"Unexpected lexical operator: " + t.token.token,
				t.token.pos, this->file_name));
		return;
	}

	// compile arguments
	for (auto& arg : t.members)
		read_tree(arg);

	std::size_t lpos = this->body.size();
	this->body.emplace_back(Command(
			Command::OPCode::BUILTIN_OP, (uint16_t) op));
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
	DLANG_DEBUG_MSG("read_macro_lit\n");

	// compile macro
	std::vector<ParsedMacro*> pps = this->parents;
	pps.emplace_back(this);
	auto* mac = new ParsedMacro(
			tree, this->file_name, pps, this->compiler,
			this->declarations);
	for (auto& m : tree.members)
		mac->read_tree(m);

	// put it in literals header
	auto litnum = this->compiler->load_macro(*mac);

	// reference it in bc
	this->body.emplace_back(Command::OPCode::USE_LIT, litnum);

}

void ParsedMacro::read_list_lit(AST& tree) {
	DLANG_DEBUG_MSG("read_list_lit\n");
	// put elements onto stack
	for (auto& m : tree.members)
		read_tree(m);

	this->body.emplace_back(Command::OPCode::MK_LIST, (int32_t) tree.members.size());
}

void ParsedMacro::read_statements(AST& tree) {
	DLANG_DEBUG_MSG("read_statements\n");
	for (AST& statement : tree.members) {
		read_tree(statement);
		this->body.emplace_back(Command(Command::OPCode::CLEAR_STACK));
	}

	// support implicit return value...
	if (!tree.members.empty())
		// don't pop last item
		this->body.pop_back();
}

void ParsedMacro::read_obj_lit(AST& tree) {
	DLANG_DEBUG_MSG("read_obj_lit");

	// Handle {}
	if (tree.members.empty()) {
		this->body.emplace_back(Command(Command::OPCode::MK_OBJ, (int32_t) 0));
		return;
	}

	this->errors.emplace_back(SemanticError(
			"Multi-item object literals currently not supported, please use {} and then initialize members",
			tree.token.pos, this->file_name));
//	if (tree.members[0].type == AST::NodeType::COMMA_SERIES) {
//		for (auto& mem : tree.members[0].members) {
//			if (mem.type == AST::NodeType::KV_PAIR) {
//				std::string key;
//				if (mem.members[0].type == AST::NodeType::IDENTIFIER || mem.members[0].type == AST::NodeType::STR_LITERAL) {
//					key = mem.members[0].token.token;
//				}
//			}
//		}
//	}

}

void ParsedMacro::read_tree(AST& tree) {
	DLANG_DEBUG_MSG("read_tree: " <<tree.type_name() <<std::endl);
	switch (tree.type) {
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
			if (tree.token.token == "=")
				read_assignment(tree);
			else if (tree.token.token == ".")
				read_dot_op(tree);
			else
				read_operation(tree);
			return;
		case AST::NodeType::LIST:
			read_list_lit(tree);
			return;
		case AST::NodeType::MACRO:
			read_macro_lit(tree);
			return;
		case AST::NodeType::INDEX:
			read_index_op(tree);
			return;
		case AST::NodeType::INVOKE:
			read_macro_invoke(tree);
			return;
		case AST::NodeType::OBJECT:
			read_obj_lit(tree);
			return;
		default:
			this->errors.emplace_back(SemanticError(
					"Unsupported feature: " + tree.type_name(),
					tree.token.pos, this->file_name));
			return;
	}
}

// Returns a new parsed macro containing compiled contents of arg
ParsedMacro* ParsedMacro::compile_expr(AST& t) {
	auto* pm = new ParsedMacro();
	pm->parents = this->parents;
	pm->parents.emplace_back(this);
	pm->file_name = this->file_name;
	pm->compiler = this->compiler;
	pm->read_tree(t);
	return pm;
}


















//
ParsedMacro::ParsedMacro(AST &tree, std::string file_name, std::vector<ParsedMacro *> parents,
		Program* prog, std::unordered_map<std::string, MutilatedSymbol> locals):
	file_name(std::move(file_name)), parents(std::move(parents)), compiler(prog), declarations(std::move(locals))
{
	MutilatedSymbol i("i"), o("o");
	declarations["o"] = o;
	declarations["i"] = i;
	this->body.emplace_back(Command(Command::OPCode::DECL_ID, i.id));
	this->body.emplace_back(Command(Command::OPCode::DECL_ID, o.id));

}


MutilatedSymbol ParsedMacro::find_id(const std::string& name) {

	auto it = this->declarations.find(name);
	if (it != this->declarations.end()) {
		return it->second;
	}

	// check previous scopes
	for (ParsedMacro* parent : parents) {
		it = parent->declarations.find(name);
		if (it != parent->declarations.end())
			return it->second;
	}

	static const std::unordered_map<std::string, MutilatedSymbol> globals = {
			{ "empty", MutilatedSymbol("empty", 0, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "print", MutilatedSymbol("print", 1, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "input", MutilatedSymbol("input", 2, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "if", MutilatedSymbol("if", 3, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "Str", MutilatedSymbol("Str", 4, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "Num", MutilatedSymbol("Num", 5, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "vars", MutilatedSymbol("vars", 6, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "async", MutilatedSymbol("async", 7, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "import", MutilatedSymbol("import", 8, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "size", MutilatedSymbol("size", 9, MutilatedSymbol::SymbolType::CONSTANT) },
			{ "copy", MutilatedSymbol("copy", 10, MutilatedSymbol::SymbolType::CONSTANT) },
	};

	// if it's a global id use that instead...
	try {
		return globals.at(name);
	} catch (...) {
		// not found
		// use before declaration
		return MutilatedSymbol("", -1);
	}

}

int64_t ParsedMacro::declare_id(const std::string& id_name) {

	auto occ = this->declarations.find(id_name);
	if (occ == this->declarations.end()) {
		MutilatedSymbol ms = MutilatedSymbol(id_name);
		const int64_t ret = ms.id;
		declarations[id_name] = ms;
		return ret;
	}
	return 0;
}


int64_t Program::load_macro(ParsedMacro& macro) {

	// copy identifiers
	for (auto& p : macro.declarations)
		this->identifiers.emplace_back(p.second);

	return this->empl_lit(ParsedLiteral(macro));
}


void Program::load_file(const std::string& fname) {
	// parse main file
	std::ifstream file = std::ifstream(fname);

//	std::cout <<"tokenizing...";

	DLANG_DEBUG_MSG("tokenizing...");
	const auto toks = tokenize_stream(file);
	DLANG_DEBUG_MSG("done\n");


	DLANG_DEBUG_MSG("parsing...");
//	std::cout <<"parsing...";
	AST main = parse(toks);
//	std::cout <<"done\n";
	DLANG_DEBUG_MSG("done\n");

	// semantic analysis
	std::vector<SemanticError> errs = process_tree(main, fname);
	DLANG_DEBUG_MSG("After Semantics: " <<debug_AST(main) <<std::endl);

	// implicit main macro
	ParsedMacro entry(main, fname,
			std::vector<ParsedMacro*>{}, this);
	entry.read_tree(main);


	// literals.back() == main()
	this->load_macro(entry);
}

// emplace a parsed literal into literals header
// return index of parsed literal
// if its already in header send index
int64_t Program::empl_lit(ParsedLiteral&& lit) {
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