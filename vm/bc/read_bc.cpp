//
// Created by tate on 18-05-20.
//

#include <map>
#include <set>
#include <algorithm>
#include <iostream>

#include "read_bc.hpp"
#include "../literal.hpp"
#include "../../compile/command.hpp"


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
		std::unordered_map<int64_t, std::vector<int64_t>>& nested_macros,
		const std::size_t litnum)
{
	std::vector <BCInstr> body;
	std::vector <int64_t> decl_ids;
	std::set <int64_t> use_ids;

	char c;


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

			return Literal(ClosureDef(capture_ids, decl_ids, body));
		}

		if (c == BCInstr::OPCode::DECL_ID) {
			// declaring identifier : need to track declarations
			int64_t idid;
			is.read((char*) &idid, sizeof(idid));
			decl_ids.emplace_back(idid);
			body.emplace_back(BCInstr((BCInstr::OPCode)c, idid));

		} else if (c == BCInstr::OPCode::USE_ID) {
			// using identifier : might need to capture from parent scope
			int64_t idid;
			is.read((char*) &idid, sizeof(idid));
			use_ids.emplace(idid);
			body.emplace_back(BCInstr((BCInstr::OPCode)c, idid));
		} else if (c == BCInstr::OPCode::USE_LIT) {
			int64_t lid;
			is.read((char*) &lid, sizeof(lid));
			nested_macros[litnum].emplace_back(lid);
			body.emplace_back(BCInstr((BCInstr::OPCode)c, lid));
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
			} else if (t == Command::ArgType::INT32) {
				int32_t arg;
				is.read((char*)&arg, sizeof(arg));
				cmd.i = arg;
			}
			body.emplace_back(cmd);
		}
	}
}


static std::vector<int64_t> generate_capture_ids(int64_t entry, std::unordered_map<int64_t, std::vector<int64_t>>& nested_closures, std::vector<Literal>& lits) {
	if (!std::holds_alternative<ClosureDef>(lits[entry].v))
		return {};
	auto& cd = std::get<ClosureDef>(lits[entry].v);

	DLANG_DEBUG_MSG("generate_capture_ids(" <<entry <<"):\n");
	auto& ret = cd.capture_ids;
#ifdef DLANG_DEBUG
	std::cout <<"cap: ";
	for (auto e : ret)
		std::cout <<e <<", ";

	std::cout <<std::endl <<"decl: ";
#endif
//	 Note: these are sorted
	auto& decl_ids = cd.decl_ids;
#ifdef DLANG_DEBUG
	for (auto e : decl_ids)
		std::cout <<e <<", ";

	std::cout <<"\n(";
#endif
	for (int64_t lid : nested_closures[entry]) {
		const auto r = generate_capture_ids(lid, nested_closures, lits);
		std::set_difference(r.begin(), r.end(), decl_ids.begin(), decl_ids.end(), std::back_inserter(ret));
//		std::copy_if(r.begin(), r.end(), std::back_inserter(ret), [&](int64_t e){
//			return !std::binary_search(decl_ids.begin(), decl_ids.end(), e);
//		});
	}
	std::sort(ret.begin(), ret.end());
#ifdef DLANG_DEBUG
	std::cout <<")\nret: ";
	for (auto e: ret)
		std::cout <<e <<", ";
	std::cout <<std::endl;
#endif
	ret.erase(std::unique(ret.begin(), ret.end()), ret.end());

	return ret;
}


std::vector<Literal> read_lit_header(std::istream& is) {
	std::vector<Literal> ret;

	char instr = is.get();

	// used to recursively generate capture ids
	std::unordered_map<int64_t, std::vector<int64_t>> nested_closures;

	while (instr != BCInstr::OPCode::END_LIT_SECTION && instr != EOF) {

		if (instr == BCInstr::OPCode::START_LIT_STRING) {
			char c;
			std::string strlit;
			while ((c = is.get()))
				strlit += c;

			ret.emplace_back(Literal(parse_str(strlit)));

			DLANG_DEBUG_MSG("Read String Lit : " << ret.size() <<std::endl);
		} else if (instr == BCInstr::OPCode::START_LIT_MACRO){

			ret.emplace_back(capture_closure(is, nested_closures, ret.size()));
			DLANG_DEBUG_MSG("Read Macro Lit : " << ret.size() <<std::endl);
		}

		//std::cout <<instr <<" : " <<(int)instr <<std::endl;

		// fetch next instruction
		instr = is.get();
	}

	// recursively deduce closure capture Id's
	generate_capture_ids(ret.size() - 1, nested_closures, ret);

	return ret;
}