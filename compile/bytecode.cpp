//
// Created by tate on 01-05-20.
//

#include <string>
#include "bytecode.hpp"

// convert to text representation
std::string compile_text(std::vector<Command> cmds)
{
	std::string ret;
	for (Command& cmd : cmds) {
		if (cmd.is_lit())
		ret += cmd.to_text();
	}
	return ret;
}


// convert to compressed binary format
// return number of bytes stored in ret
std::size_t compile_bin(std::vector<Command>, char*& ret)
{

}
