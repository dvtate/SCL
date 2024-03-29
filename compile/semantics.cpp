//
// Created by tate on 02-05-20.
//

#include <filesystem>
#include <cstring>
#include <cstdlib>
#include <istream>
#include <fstream>

#include "semantics.hpp"

/// Represents a file that has been imported
class ImportedFile {
public:
	/// Location of file
	std::filesystem::path path;

	/// This Cstring version of path will be same pointer used in all tokens from this file
	/// So keeping it here in case it's of use later
	const char* absolute_path_str;

	/// Code contained in file
	std::shared_ptr<AST> tree;

	ImportedFile(const char* absolute_path, std::filesystem::path path, std::shared_ptr<AST> tree):
		path(std::move(path)), absolute_path_str(absolute_path), tree(std::move(tree))
	{}
};

/// This class only really exists to capture state associated with the semantic analysis...
///   Really should just be some hidden functions
class SemanticProcessor {
protected:
	/// Imports
	std::vector<ImportedFile> imports;

	/**
	 * Import a source file
	 * @param t - token of import
	 * @param file_name - import file name
	 * @return - -1 on error otherwise the index of the import object in this->imports
	 */
	ssize_t import(const Token& t, const std::string& file_name) {
		// TODO something like node_modules/ or $PATH or something
		if (file_name[0] != '.') {
			this->add_error(SemanticError("Builtin/System imports not currently supported", t));
			return -1;
		}

		// Locate the file
		std::filesystem::path p;
		try {
			p = std::filesystem::canonical(std::filesystem::path(file_name));
		} catch (const std::filesystem::filesystem_error& e) {
			this->add_error(SemanticError(std::string("Invalid import, \"") + file_name + '"', t));
			return -1;
		}

		// They're trying to import a .so file probably
		if (p.extension() != ".scl") {
			return -2;
		}

		// Check if we've imported it already
		const auto it = std::find_if(this->imports.begin(), this->imports.end(), [&] (const ImportedFile& f) {
			return f.path == p;
		});

		// Already been imported
		if (it != this->imports.end())
			return it - this->imports.begin();

		// Copy the path into a C string
		const auto len = p.string().length() + 1;
		char* full_path_name = (char*) malloc(len);
		memcpy(full_path_name, p.string().c_str(), len);

		// Parse
		SCL_DEBUG_MSG("import: lexing...")
		auto is = std::ifstream(p);
		auto tokens = tokenize_stream(is, full_path_name);
		SCL_DEBUG_MSG("done\n")

		SCL_DEBUG_MSG("import: parsing...")
		AST tree = parse(tokens);
		SCL_DEBUG_MSG("done\n")

		// Generate hiden identifier
		const ssize_t ret = this->imports.size();
		std::string hidden_id = import_hidden_identifier(ret);

		// Wrap file in a closure and assign it to a global hidden identifier
		// All future imports of this module will refer to this identifier
		// ie: let $(" import $FILE") = (: ... file contents )()
		AST assignment = AST(AST::NodeType::DECLARATION, Token(Token::t::IDENTIFIER, "let", full_path_name), {
			AST(AST::NodeType::OPERATION, Token(Token::t::OPERATOR, "=", full_path_name), {
				AST(AST::NodeType::IDENTIFIER, Token(Token::t::IDENTIFIER, hidden_id, full_path_name)),
				AST(AST::NodeType::INVOKE, Token(Token::t::OPERATOR, "@", full_path_name), {
					AST(AST::NodeType::MACRO, Token(Token::t::OPERATOR, "(:)", full_path_name), { tree }),
				}),
			}),
		});

		// Do semantics on that tree too
		const auto cp = std::filesystem::current_path();
		std::filesystem::current_path(p.parent_path());
		assignment = this->process_tree(assignment);
		std::filesystem::current_path(cp);

		//
		this->imports.emplace_back(ImportedFile(full_path_name, p, std::make_shared<AST>(assignment)));

		return ret;
	}

	inline AST& convert_syntax_branch(AST& t);
	static inline AST& convert_syntax_leaf(AST& t) { return t; }

	static inline std::string import_hidden_identifier(const size_t index) {
		if (index == 0)
			return "empty";
		return std::string(" import ") + std::to_string(index);
	}

public:
	AST* head;

	explicit SemanticProcessor(AST* head): head(head) {
		// Need to be able to expect that top level consists of statements
		if (head->type != AST::NodeType::STATEMENTS)
			*head = AST(AST::NodeType::STATEMENTS, head->token, { *head });

		// Import zero special
		this->imports.emplace_back(
				ImportedFile(head->token.file,
				head->token.file,
				std::make_shared<AST>()));

		// Put import declarations at start of file
		*head = process_tree(*head);
		std::vector<std::shared_ptr<AST>> mems;
		for (unsigned i = 1; i < this->imports.size(); i++) {
			mems.emplace_back(this->imports[i].tree);
		}
		for (auto&& m : head->members) {
			mems.emplace_back(m);
		}
		head->members = mems;
	}

	AST& process_tree(AST& t) {
		return this->convert_syntax_branch(t);
	}

	/// Errors and Warnings
	std::vector<SemanticError> errs;

	void add_error(SemanticError&& error) {
		this->errs.emplace_back(error);
	}
};


/**
* - Convert Builtin-identifiers/constants (ie - print)
* - Handle operator associativity
* - Convert complex let expressions
* 		+ let a = 2, b = 4;
* 		--->
* 		+ let a; a = 2; let b; b = 4;
* - remove PAREN_EXPRs
* - Convert function calls on CSV's to list
*/

/// Finish up job of parser
AST& SemanticProcessor::convert_syntax_branch(AST& t) {
	if (t.type != AST::NodeType::PAREN_EXPR && t.members.empty())
		return SemanticProcessor::convert_syntax_leaf(t);

	// convert paren exprs
	if (t.type == AST::NodeType::PAREN_EXPR) {
		if (t.members.size() > 1) {
			this->add_error(SemanticError("Invalid parenthesized expression", t.token.pos));
			return t;
		}
		return convert_syntax_branch(*t.members[0]);
	}

	// Handle import function
	if (t.type == AST::NodeType::INVOKE) {
		const auto& fxn = t.members[0];
		if (fxn->type == AST::NodeType::IDENTIFIER && fxn->token.token == "import") {
			const auto& file = t.members[1];
			if (file->type != AST::NodeType::STR_LITERAL) {
				this->add_error(SemanticError("import() expected a string literal", file->token.pos));
			} else {
				const auto idx = this->import(file->token, file->token.token);
				if (idx >= 0) {
					// Note this makes
					t.type = AST::NodeType::IDENTIFIER;
					t.token.type = Token::t::IDENTIFIER;
					t.token.token = import_hidden_identifier(idx);
					t.members.clear();
					return SemanticProcessor::convert_syntax_leaf(t);
				}
			}
		}
	}

	// Convert comma operator to COMMA_SERIES
	if (t.type == AST::NodeType::OPERATION && t.token.token == ",")
		t.type = AST::NodeType::COMMA_SERIES;

	// associativity... ideally would have been handled by parser :(
	if (t.type == AST::NodeType::OPERATION && t.members.size() > 2) {
		const std::string& op_sym = t.token.token;

		if (op_sym == ":=" || op_sym == "=" || op_sym == "**") {
			// right associative operators (a = (b = 5))
			AST mem = t;
			mem.members.clear();
			mem.members.emplace_back(t.members.back());
			t.members.pop_back();
			for (unsigned i = t.members.size(); i > 0; i--) {
				mem.members.emplace_back(t.members[i]);
				AST tmp = (mem);
				mem = t;
				mem.members.clear();
				mem.members.emplace_back(std::make_shared<AST>(tmp));
			}
			mem.members.emplace_back(t.members[0]);
			t = mem;

		} else if (op_sym == "@" || op_sym == ",") {
			// non-associative
			// no action
		} else {
			// left-associative 1+2+3 => ((1+2)+3)
			// Populate from right to left
			const auto n = AST(AST::NodeType::OPERATION, t.token);
			auto ret = std::make_shared<AST>(AST::NodeType::OPERATION, t.token);
			auto cur = ret;
			for (auto i = t.members.size() - 1; i > 1; i--) {
				cur->members.push_back(std::make_shared<AST>(n));
				cur->members.push_back(t.members[i]);
				cur = cur->members[0];
			}
			cur->members.push_back(t.members[0]);
			cur->members.push_back(t.members[1]);
			t = *ret;
		}
	}

	if (t.members.empty())
		return SemanticProcessor::convert_syntax_leaf(t);

	// map members
	AST& ret = t;
	for (unsigned int i = 0; i < t.members.size(); i++) {
		*ret.members[i] = this->convert_syntax_branch(*t.members[i]);
	}
	return ret;
}


std::vector<SemanticError> process_tree(AST& tree) {
	auto sp = SemanticProcessor(&tree);
	return sp.errs;
}
