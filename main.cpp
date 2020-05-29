#include <unistd.h>
#include <getopt.h>
#include <iostream>

#include "parse/lex.hpp"
#include "parse/parse.hpp"
#include "compile/compile.hpp"
#include "compile/bytecode.hpp"
#include "util.hpp"
#include "vm/vm.hpp"
#include "vm/read_bc.hpp"

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

	run = out_bc = out_bct = false;
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
	}

	if (run) {
		std::ifstream bc_src = std::ifstream(fname);
#ifdef DLANG_DEBUG
		std::cout <<"reading lit header... ";
#endif
		std::vector<Literal>&& lits = read_lit_header(bc_src);

#ifdef DLANG_DEBUG
		std::cout <<"done\n";
#endif
		// TODO: capture argv...

		VM interpreter{lits, { "argv not implemented" }};
		interpreter.run();
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
				std::cout <<"Compiler Warning: " <<e.msg <<std::endl
						  << util::show_line_pos(fis, e.pos, fname);
			} else {
				std::cout <<"Compiler Error: " <<e.msg <<std::endl
						<<util::show_line_pos(fis, e.pos, fname) <<std::endl;
				fatal = true;
			}

		if (fatal)
			return 1;
	}

	if (out_bc) {
		std::ofstream out{"o.bin"};
		for (const char c : compile_bin(bytecode)) {

		}
	}
	if (out_bct)
		std::cout <<compile_text(bytecode) <<std::endl;

//	std::cout <<"empty:\t"	<<get_index<Value::empty_t, Value::variant_t>() <<std::endl;
//	std::cout <<"float:\t"	<<get_index<Value::float_t, Value::variant_t>() <<std::endl;
//	std::cout <<"int: \t"	<<get_index<Value::int_t, Value::variant_t>() <<std::endl;
//	std::cout <<"str: \t"	<<get_index<Value::str_t, Value::variant_t>() <<std::endl;
//	std::cout <<"ref: \t"	<<get_index<Value::ref_t, Value::variant_t>() <<std::endl;
//	std::cout <<"lam: \t"	<<get_index<Value::lam_t, Value::variant_t>() <<std::endl;
//	std::cout <<"nfn: \t"	<<get_index<Value::n_fn_t, Value::variant_t>() <<std::endl;
//
//	class ff : public virtual NativeFunction {
//		void operator()(Frame& f) override { }
//	} nfn;
//
//	Value::variant_t v0, v1(1.1), v2(1000L), v3("fff"),
//		v4(Handle(new Handle(new Value()))), v5(Closure{}), v6(Handle<NativeFunction>{& nfn});
//
//
//	std::cout
//		<<v0.index()
//		<<v1.index()
//		<<v2.index()
//		<<v3.index()
//		<<v4.index()
//		<<v5.index()
//		<<v6.index();


}