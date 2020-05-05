
#include <unistd.h>
#include <getopt.h>
#include <iostream>

#include "parse/lex.hpp"
#include "parse/parse.hpp"
#include "compile/compile.hpp"
#include "compile/bytecode.hpp"

// dlang <cmd> <in> options

void print_help_msg(){
	std::cout <<"For now these are options:\n"
	   " -c : compile\n"
	   " -r : run\n"
	   " -o : output bytecode text\n"
	   " -O : output compressed bytecode\n"
	   " -f : required input file\n";
}

int main(int argc, char** argv) {
	bool run, out_bc, out_bct;
	char* fname = nullptr;

	/* For now these are options
	 * -c : compile
	 * -r : run(let return right)

	 * -o : output bytecode text
	 * -O : output compressed bytecode
	 * -f : required input file
	 */

	int opt;
	while ((opt = getopt(argc, argv, "croOf:"))) {
		int b = false;
		switch (opt) {
		case 'r':
			run = true;
			break;
		case 'o':
			out_bc = true;
			break;
		case 'O':
			out_bct = true;
			break;
		case 'f':
			fname = optarg;
			break;
		default:
			b = true;
			break;
		}
		if (b) break;
	}

	// they want a shell
	if (fname == nullptr || !(run || out_bc || out_bct)) {

	}

	Program p(fname);
	std::vector<Command> bytecode;
	std::vector<SemanticError> errs = p.compile(bytecode);

	if (out_bc)
		for (const char c : compile_bin(bytecode))
			std::cout <<c;
	if (out_bct)
		std::cout <<compile_text(bytecode) <<std::endl;

}