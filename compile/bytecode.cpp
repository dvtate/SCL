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
// return number of bytes stored in ret
std::vector<char> compile_bin(std::vector<Command> cmds)
{
	// this will have horrible performance...
	std::vector<char> ret;
	char* buff =
}
