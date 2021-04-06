//
// Created by tate on 01-05-20.
//

#include <string>
#include "bytecode.hpp"

// convert to text representation
std::string compile_text(std::vector<Command> cmds)
{
	std::string ret;
	size_t lit_num = 0;
	for (Command& cmd : cmds) {
		if (cmd.is_lit())
			ret += std::string("# Literal ") + std::to_string(lit_num++) + ":\n";
		ret += cmd.to_text();
	}
	return ret;
}

// convert to compressed binary format
std::vector<char> compile_bin(std::vector<Command> cmds)
{
	std::vector<char> ret;
	char* buff = (char*) malloc(50);
	int n;
	for (Command& cmd : cmds) {
		n = cmd.compile(buff);
		for (int i = 0; i < n; i++)
			ret.push_back(buff[i]);
	}
	return ret;
}
