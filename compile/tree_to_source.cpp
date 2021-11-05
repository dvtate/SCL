//
// Created by tate on 20/04/2021.
//

#include "tree_to_source.hpp"


//
#ifdef SCL_DEBUG_MODE
static constexpr bool is_debug_mode = true;
#else
static constexpr bool is_debug_mode = false;
#endif

// Note that this is also used for COMMA_SERIES
static inline std::string src_operator(AST& tree) {
	const auto& op = tree.token.token;

	// Handle order of operations
	const auto operand = [&](AST& t) {
		if (is_debug_mode || (t.type == AST::OPERATION && operator_precedence[op] > operator_precedence[t.token.token]))
			return "(" + tree_to_source(t) + ")";
		else
			return tree_to_source(t);
	};

	// This should never happen
	if (tree.members.empty()) {
		std::cout <<"tree_to_src: wtf operation with no operands\n";
		return "(" + op + ")";
	}

	// Unary prefix
	if (op == "!")
		return "!" + operand(*tree.members[0]);
	if (op == "neg")
		return "-" + operand(*tree.members[0]);

	// Join on op
	std::string ret = operand(*tree.members[0]);
	for (int i = 1; i < tree.members.size(); i++)
		ret += op + operand(*tree.members[i]);
	return ret;
}

static inline std::string src_statements(AST& tree) {
	std::string ret;
	for (auto& m : tree.members)
		ret += tree_to_source(*m) + ";";
	return ret;
}

static inline std::string src_macro(AST& tree) {
	if (tree.members.empty())
		return "(:)";

	std::string ret = "(:" + tree_to_source(*tree.members[0]);
	for (unsigned i = 1; i < tree.members.size(); i++)
		ret += (is_debug_mode ? "\n" : " ") + tree_to_source(*tree.members[i]);
	return ret + ")";
}

std::string tree_to_source(AST& tree) {
	switch (tree.type) {
		case AST::NodeType::OPERATION:
		case AST::NodeType::COMMA_SERIES:
			return src_operator(tree);
		case AST::NodeType::IDENTIFIER:
		case AST::NodeType::NUM_LITERAL:
			return tree.token.token;
		case AST::NodeType::STR_LITERAL:
			return "\"" + tree.token.token + "\"";

		case AST::NodeType::MACRO:
			return src_macro(tree);

		case AST::NodeType::LIST:
			if (tree.members.size() > 1)
				std::cerr <<"tree_to_src: bad list - " <<tree.members.size() <<std::endl;
			return "[" + tree_to_source(*tree.members[0]) + "]";

		case AST::NodeType::OBJECT:
			if (tree.members.size() > 1)
				std::cerr <<"tree_to_src: bad object - " <<tree.members.size() <<std::endl;
			if (tree.members.empty())
				return "{}";
			return "{" + tree_to_source(*tree.members[0]) + "}";

		case AST::NodeType::KV_PAIR:
			return tree_to_source(*tree.members[0]) + ":" + tree_to_source(*tree.members[1]);
		case AST::NodeType::DECLARATION:
			return "let " + tree_to_source(*tree.members[0]);
		case AST::NodeType::STATEMENTS:
			return src_statements(tree);
		case AST::NodeType::INVOKE:
			if (tree.members.size() == 1)
				return tree_to_source(*tree.members[0]) + "()";
			else
				return tree_to_source(*tree.members[0]) + "(" + tree_to_source(*tree.members[1]) + ")";
		case AST::INDEX:
			return tree_to_source(*tree.members[0]) + "[" + tree_to_source(*tree.members[1]) + "]";

//		case AST::NodeType::OPERATOR:
//		case AST::NodeType::CONT_CLOSE:
//		case AST::NodeType::LIST_OPEN:
//		case AST::NodeType::MACRO_OPEN:
//		case AST::NodeType::PAREN_OPEN:
//		case AST::NodeType::PAREN_EXPR:
//		case AST::NodeType::OBJ_OPEN:
//		case AST::NodeType::INVALID:
		default:
			return "{{invalid : " + tree.type_name() + "}}";
	}

}