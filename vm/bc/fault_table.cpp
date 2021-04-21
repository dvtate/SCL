//
// Created by tate on 10/04/2021.
//
#include <iostream>

#include "bc.hpp"

#include "fault_table.hpp"

FaultTable* FaultTable::read(std::istream& is) {
	auto* ret = new FaultTable;

	char c = is.get();

	std::shared_ptr<std::string> file;
	std::string str;
	uint64_t id;

	while (c != EOF) {
		switch (c) {
			// Read string symbol
			case BCInstr::OPCode::ID_NAME:
			case BCInstr::OPCode::INVOKE_REPR:
				str = "";
				while ((c = is.get()))
					str += c;
				break;

			// Mutilated id
			case BCInstr::OPCode::ID_ID:
				is.read((char*) &id, sizeof(id));
				ret->source_ids[id] = str;
				break;


			// depictions of invoked values (relevant position)
			case BCInstr::OPCode::INVOKE_POS:
				is.read((char*) &id, sizeof(id));
				ret->invoke_lhs[id] = str;
				break;

			// Relocations
			case BCInstr::OPCode::FILE_NAME:
				str = "";
				while ((c = is.get()))
					str += c;
				file = std::make_shared<std::string>(str);
				break;
			case BCInstr::OPCode::DEST_POS:
				is.read((char*) &id, sizeof(id));
				break;
			case BCInstr::OPCode::SRC_POS: {
				uint64_t src_pos;
				is.read((char*) &src_pos, sizeof(src_pos));
				ret->relocations[id] = std::pair<std::shared_ptr<std::string>, uint64_t>{ file, src_pos };
				break;
			}

			default:
				std::cerr <<"unexpected items in the fault table\n";
		}
	}

	return ret;
}