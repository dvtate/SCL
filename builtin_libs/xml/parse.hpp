//
// Created by tate on 17/06/2021.
//

#ifndef SCL_PARSE_HPP
#define SCL_PARSE_HPP

#include <optional>

#include "../../vm/vm.hpp"
#include "../../vm/error.hpp"

/// Exported native function
static NativeFunction* parse_nfn;

class XMLParseFn : public NativeFunction {
	/// Thrown on invalid XML input
	class ParseError : public std::exception {
	public:
		// Character index
		size_t pos;

		// Why it's bad
		std::string message;

		ParseError(size_t pos, std::string message):
			pos(pos), message(std::move(message))
		{}
	};

    struct Node {
        std::string tag;
        std::vector<std::variant<Node, std::string>> members;
        std::unordered_map<std::string, std::string> attributes;

        Node(std::string tag): tag(std::move(tag)) {}
    };


	/// Skip whitespaces
	static void ws(const std::string& s, size_t& i) {
		while (i < s.size() && isspace(s[i]))
			if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r')
				return;
			else
				i++;
	}

    static std::optional<Node> parse_xml(std::string& s, size_t& i) {
        std::deque<Node> tags;

        ws(s, i);
        if (s[i] != '<') {
            throw ParseError(i, "expected <");
        }
    }
};