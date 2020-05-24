//
// Created by tate on 18-05-20.
//

#include <map>
#include <set>
#include <algorithm>
#include <iostream>
#include "read_bc.hpp"
#include "literal.hpp"
#include "../compile/command.hpp"

//
static inline void ignore_until(std::istream& is, BCInstr::OPCode instr) {
	char c;
	for (;;) {
		is >>c;
		if (c == instr)
			return;
		is.ignore(8);
	}
}


// convert escaped characters into proper ascii codes
static inline std::string parse_str(std::string in) {
	// TODO: implement (can steal code from ys)
	return in;
}


static inline Literal capture_closure(std::istream& is,
		std::unordered_map<int64_t, std::vector<int64_t>>& nested_macros) {
	std::vector <BCInstr> body;
	std::vector <int64_t> decl_ids;
	std::set <int64_t> use_ids;

	int64_t i_id = 0, o_id = 0;

	char c;

	// read i
	c = is.get();
	if (c != BCInstr::OPCode::DECL_ID) {
		std::cout <<"expected i_id at start of macro lit\n";
		is.unget();
	} else {
		is.read((char*) &i_id, sizeof(i_id));
	}

	// read o
	c = is.get();
	if (c != BCInstr::OPCode::DECL_ID) {
		std::cout <<"expected o_id at start of macro lit\n";
		is.unget();
	} else {
		is.read((char*) &o_id, sizeof(o_id));
	}

	while (true) {

		c = is.get();

		// break condition
		if (c == BCInstr::OPCode::END_LIT_MACRO) {

			// don't need to capture variables declared within macro scope
//			std::erase_if(use_ids, [&decl_ids](int64_t id){
//				decl_ids.contains(id);
//			});
			std::vector<int64_t> capture_ids;
			std::set_difference(use_ids.begin(), use_ids.end(),
					decl_ids.begin(), decl_ids.end(),
					std::inserter(capture_ids, capture_ids.begin()));

			return Literal(ClosureDef(capture_ids, decl_ids, body, i_id, o_id));
		}

		if (c == BCInstr::OPCode::DECL_ID) {
			// declaring identifier : need to track declarations
			int64_t idid;
			is.read((char*) &idid, sizeof(idid));
			decl_ids.emplace_back(idid);

		} else if (c == BCInstr::OPCode::USE_ID) {
			// using identifier : might need to capture from parent scope
			int64_t idid;
			is.read((char*) &idid, sizeof(idid));
			use_ids.emplace(idid);
			BCInstr cmd{};
			cmd.instr = (BCInstr::OPCode) c;
			cmd.i = idid;
			body.emplace_back(cmd);
		} else {
			// generic instruction
			BCInstr cmd{};
			cmd.instr = (BCInstr::OPCode) c;
			const Command::ArgType t = Command::arg_type((BCInstr::OPCode) c);
			if (t == Command::ArgType::INT64) {
				int64_t iv;
				is.read((char*)&iv, sizeof(iv));
				cmd.i = iv;
			} else if (t == Command::ArgType::FLOAT) {
				double arg;
				is.read((char*)&arg, sizeof(arg));
				cmd.v = arg;
			} else if (t == Command::ArgType::INT16) {
				int16_t arg;
				is.read((char*)&arg, sizeof(arg));
				cmd.i = arg;
			}

			body.emplace_back(cmd);
		}
	}
}


std::vector<Literal> read_lit_header(std::istream& is) {
	std::vector<Literal> ret;

	char instr;
	is >> instr;

	// used to recursively generate capture ids
	std::map<int64_t, std::vector<int64_t>> nested_closures;

	while (instr != BCInstr::OPCode::END_LIT_SECTION) {

		if (instr == BCInstr::OPCode::START_LIT_STRING) {
			char c;
			std::string strlit;
			do {
				c = is.get();
				if (c)
					strlit += c;
				else
					break;
			} while (c);

			ret.emplace_back(Literal(Value(parse_str(strlit))));
		} else if (instr == BCInstr::OPCode::START_LIT_MACRO){

		}

		// fetch next instruction
		is >> instr;
	}

}