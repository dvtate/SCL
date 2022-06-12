#include <iostream>

#include "parse/lex.hpp"
#include "parse/parse.hpp"
#include "compile/compile.hpp"
#include "compile/bytecode.hpp"
#include "util.hpp"
#include "vm/vm.hpp"
#include "vm/bc/read_bc.hpp"
#include "compile/tree_to_source.hpp"

// scl <cmd> <in> options

void print_help_msg(){
	std::cout <<"scl <command> [flags]\n"
		<<"\nUsage:\n"
		<<"scl build <entry file>    # Generate bytecode binary\n"
		<<"scl minify <entry file>   # Minify the input code\n"
		<<"scl debug <entry file>    # Generate bytecode text\n"
		<<"scl exec <bytecode file>  # Execute bytecode\n"
		<<"scl eval <entry file>     # Build and Exect programs\n";

#ifndef SCL_DEBUG_MSG
	std::cout <<"\nRecompile with debugging enabled for verbose output\n";
#else
	std::cout <<"\nYou are using the debug version of SCL\n";
#endif
}

// The main virtual machine instance
VM* interpreter;

int main(int argc, char** argv) {
	// Not enough arguments
	if (argc < 3) {
		print_help_msg();
		return 1;
	}

	const char* cmd = argv[1];
	const char* arg = argv[2];
	if (!arg) {
		print_help_msg();
		return 1;
	}

	bool exec, out_bc, out_bct, out_minified, eval;
	exec = out_bc = out_bct = out_minified = eval = false;

	if (strcmp(cmd, "build") == 0)
		out_bc = true;
	else if (strcmp(cmd, "minify") == 0)
		out_minified = true;
	else if (strcmp(cmd, "debug") == 0)
		out_bct = true;
	else if (strcmp(cmd, "exec") == 0)
		exec = true;
	else if (strcmp(cmd, "eval") == 0)
		eval = true;
	else {
		std::cerr <<"unknown command \"" <<cmd <<"\"\n";
		print_help_msg();
	}

	if (exec) {
		std::ifstream bc_src = std::ifstream(arg);
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
		interpreter = new VM{lits, args, bc_src};
		interpreter->run();
		return 0; // never gets called... (hopefully)
	}

	Program p;
	try {
		p = Program(arg);
	} catch (std::vector<SyntaxError>& es) {
		for (auto& e : es)
			std::cout <<"Syntax Error: " <<e.msg <<std::endl
				<<util::show_line_pos(arg, e.token.pos) <<std::endl;
	}
	std::vector<Command> bytecode;
	std::vector<SemanticError> errs = p.compile(bytecode);

	if (!errs.empty()) {
		bool fatal = false;
		auto fis = std::ifstream(arg); // reuse istream
		for (auto& e : errs)
			if (e.is_warn) {
				std::cout <<util::term_eff_red <<"Compiler Warning: " <<util::term_eff_reset <<e.msg <<std::endl
						  << util::show_line_pos(fis, e.pos, e.file.string());
			} else {
				std::cout <<util::term_eff_red <<"Compiler Error: " <<util::term_eff_reset <<e.msg <<std::endl
						<<util::show_line_pos(fis, e.pos, e.file.string()) <<std::endl;
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
	if (out_minified) {
		std::cout <<tree_to_source(p.main) <<std::endl;
	}

	if (eval) {
		// Compile file
		std::stringstream bin{"o.bin"};
		for (const char c : compile_bin(bytecode))
			bin << c;

		// Run file
#ifdef SCL_DEBUG
		std::cout <<"reading lit header... ";
#endif
		std::vector<Literal>&& lits = read_lit_header(bin);

#ifdef SCL_DEBUG
		std::cout <<"done\n";
#endif
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; i++)
			args.emplace_back(argv[i]);
		interpreter = new VM{lits, args, bin};
		interpreter->run();
		return 0; // never gets called... (hopefully)
	}
}