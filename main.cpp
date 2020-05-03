
#include <unistd.h>
#include <getopt.h>
#include <iostream>

#include "parse/lex.hpp"
#include "parse/parse.hpp"
#include "compile/compile.hpp"


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
//	bool compile, run, out_bc, out_bct;
//	char* fname = nullptr;
//
//	/* For now these are options
//	 * -c : compile
//	 * -r : run(let return right)
//
//	 * -o : output bytecode text
//	 * -O : output compressed bytecode
//	 * -f : required input file
//	 */
//
//	int opt;
//	while ((opt = getopt(argc, argv, "croOf:"))) {
//		switch (opt) {
//		case 'c':
//			compile = true;
//			break;
//		case 'r':
//			run = true;
//			break;
//		case 'o':
//			out_bc = true;
//			break;
//		case 'O':
//			out_bct = true;
//			break;
//		case 'f':
//			fname = optarg;
//			break;
//		default:
//			print_help_msg();
//			exit(-1);
//		}
//	}
//
//	// they want a shell
//	if (fname == nullptr || !(run || compile || out_bc || out_bct)) {
//
//	}

	Program(std::string(argv[1]));
}