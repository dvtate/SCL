#include <unistd.h>
#include <getopt.h>
#include <iostream>

#include "parse/lex.hpp"
#include "parse/parse.hpp"
#include "compile/compile.hpp"
#include "compile/bytecode.hpp"
#include "util.hpp"
#include "vm/vm.hpp"
#include "vm/bc/read_bc.hpp"

// scl <cmd> <in> options

void print_help_msg(){
	std::cout <<"For now these are options:\n"
	   " -c : compile\n"
	   " -r : run\n"
	   " -o : output bytecode text\n"
	   " -O : output compressed bytecode\n"
	   " -f : required input file\n";
}

VM* interpreter;


int main(int argc, char** argv) {
	/* For now these are options
	 * -c : compile
	 * -r : run(let return right)

	 * -o : output bytecode text
	 * -O : output compressed bytecode
	 * -f : required input file
	 */

	bool run, out_bc, out_bct;
	run = out_bc = out_bct = false;
	char* fname = nullptr;
	int opt;
	while ((opt = getopt(argc, argv, "croOf:"))) {
		int b = false;
		switch (opt) {
		case 'r':
			run = true;
			break;
		case 'o':
			out_bct = true;
			break;
		case 'O':
			out_bc = true;
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
		print_help_msg();
		return 1;
	}

	if (run) {
		std::ifstream bc_src = std::ifstream(fname);
#ifdef SCL_DEBUG
		std::cout <<"reading lit header... ";
#endif
		std::vector<Literal>&& lits = read_lit_header(bc_src);

#ifdef SCL_DEBUG
		std::cout <<"done\n";
#endif
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; i++)
			args.emplace_back(argv[i]);
		interpreter = new VM{lits, args};
		interpreter->run();
		return 0; // never gets called... (hopefully)
	}

	Program p;
	try {
		p = Program(fname);
	} catch (std::vector<SyntaxError>& es) {
		for (auto& e : es)
			std::cout <<"Syntax Error: " <<e.msg <<std::endl
				<<util::show_line_pos(fname, e.token.pos) <<std::endl;
	}
	std::vector<Command> bytecode;
	std::vector<SemanticError> errs = p.compile(bytecode);

	//
	if (!errs.empty()) {
		bool fatal = false;
		auto fis = std::ifstream(fname); // reuse istream
		for (auto& e : errs)
			if (e.is_warn) {
				std::cout <<util::term_eff_red <<"Compiler Warning: " <<util::term_eff_reset <<e.msg <<std::endl
						  << util::show_line_pos(fis, e.pos, fname);
			} else {
				std::cout <<util::term_eff_red <<"Compiler Error: " <<util::term_eff_reset <<e.msg <<std::endl
						<<util::show_line_pos(fis, e.pos, fname) <<std::endl;
				fatal = true;
			}

		if (fatal)
			return 1;
	}

	if (out_bc) {
		std::ofstream out{"o.bin"};
		for (const char c : compile_bin(bytecode))
			out << c;

		std::cout <<"compiled to o.bin\n";
	}
	if (out_bct)
		std::cout <<compile_text(bytecode) <<std::endl;


}