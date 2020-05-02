//
// Created by tate on 29-04-20.
//

#ifndef DLANG_COMMAND_HPP
#define DLANG_COMMAND_HPP

#include <cinttypes>
#include <cstdlib>
#include <variant>
#include <cstring>


class Command {
public:

	// mnemonic instruction
	enum OPCode : char {
		END_LIT_STRING = 0,
		// these go in lit header
		START_LIT_STRING = 1,
		START_LIT_MACRO,
		START_LIT_JSON,
		//END_LIT_STRING,		// just use '\0'...
		END_LIT_MACRO,
		//END_LIT_JSON,			// just use '\0'

		END_LIT_SECTION,

		// these can go in macro bodies
		I64_LIT,		// push int literal onto stack (too small for header)
			// arg: int64
		F64_LIT,		// float literal (too small for header)
			// arg: double
		DECL_ID,		// declare identifier
			// arg: int64, idid
		USE_ID,			// reference identifier
			// arg: int64, idid
		USE_LIT,		// push literal onto stack
			// arg: int64, lit id
		BUILTIN_OP,		// operate on stack
			// arg: int16, builtin operator id
		KW_VAL, 		// builtin keyword-literal (ie- print)
			// arg: int16

		// begin fault table
		ID_NAME,		// user-defined identifier name
			// arg: str
		ID_ID, 			// compiled identifier number
			// arg: int64
		FILE_NAME,		// file path
			// arg: str
		DEST_POS,		// location in compiled file
			// arg: int64
		SRC_POS,		// original file position
			// arg: int64

	} instr;

	std::variant<uint16_t, int64_t, double, std::string> arg;

	enum ArgType {
		INT64, INT16, FLOAT, STRING, NO_ARG
	};

	// used for generating bytecode text format for debugging
	inline std::string to_text() {
		switch (this->instr) {

			// literal defs
			case OPCode::START_LIT_STRING:
				return "String: \"" + std::get<std::string>(this->arg) + "\"\n";
			case OPCode::START_LIT_JSON:
				return "JSON: " + std::get<std::string>(this->arg) + "\n";
			case OPCode::START_LIT_MACRO:
				return "Macro: (:\n";
			case OPCode::END_LIT_MACRO:
				return ")\n";
			case OPCode::END_LIT_SECTION:
				return "#### End Literal Section ####\n";

				// indented bc must be within macro body
			case OPCode::I64_LIT:
				return "\tint64 " + std::to_string(std::get<int64_t>(this->arg));
			case OPCode::F64_LIT:
				return "\tdouble " + std::to_string(std::get<double>(this->arg));
			case OPCode::DECL_ID:
				return "\tlet " + std::to_string((uint64_t) std::get<int64_t>(this->arg));
			case OPCode::USE_ID:
				return "\tid $" + std::to_string((uint64_t) std::get<int64_t>(this->arg));
			case OPCode::USE_LIT:
				return "\tliteral #" + std::to_string((uint64_t) std::get<int64_t>(this->arg));
			case OPCode::BUILTIN_OP:
				// switch (std::get<int16_t>(this->arg)) { ... }
				return "\tOP #" + std::to_string(std::get<uint16_t>(this->arg));
			case OPCode::KW_VAL:
				// switch (std::get<int16_t>(this->arg)) { ... }
				return "\tKW_LIT #" + std::to_string(std::get<uint16_t>(this->arg));


				// dont care about fault table yet
			default:
				return "";
		}
	}

	// check if it goes in literal header
	inline bool is_lit() const noexcept {
		return this->instr < OPCode::END_LIT_SECTION;
	}

	inline ArgType arg_type() {
		switch (this->instr) {
			case OPCode::F64_LIT:
				return ArgType::FLOAT;
			case OPCode::I64_LIT: case OPCode::DECL_ID: case OPCode::USE_ID: case OPCode::USE_LIT:
			case OPCode::ID_ID: case OPCode::SRC_POS: case OPCode::DEST_POS:
				return ArgType::INT64;
			case OPCode::BUILTIN_OP: case OPCode::KW_VAL:
				return ArgType::INT16;
			case OPCode::START_LIT_STRING: case OPCode::START_LIT_JSON: case OPCode::ID_NAME: case OPCode::FILE_NAME:
				return ArgType::STRING;

			default:
				return ArgType::NO_ARG;
		}
	}

	// copies compiled instruction into ret
	// returns number of chars to read
	std::size_t compile(char*& ret) {
		ArgType t = arg_type();
		size_t s = 1;
		switch (t) {
			case ArgType::STRING: {
				auto &str = std::get<std::string>(this->arg);
				s += str.size() + 1;
				ret = (char *) realloc(ret, s);
				strncpy(ret, (char *) &this->instr, 1);
				strcpy(ret + 1, str.c_str()); // also copies the '\0'
				break;
			}
			case ArgType::INT16: {
				s += sizeof(uint16_t);
				ret = (char *) realloc(ret, s);
				uint16_t n = std::get<uint16_t>(this->arg);
				strncpy(ret, (char *) &this->instr, 1);
				strncpy(ret + 1, (char *) &n, sizeof(uint16_t));
				break;
			}
			case ArgType::INT64: {
				s += sizeof(int64_t);
				ret = (char *) realloc(ret, s);
				int64_t n = std::get<int64_t>(this->arg);
				strncpy(ret, (char *) &this->instr, 1);
				strncpy(ret + 1, (char *) &n, sizeof(int64_t));
				break;
			}
			case ArgType::FLOAT: {
				s += sizeof(double);
				ret = (char *) realloc(ret, s);
				double n = std::get<double>(this->arg);
				strncpy(ret, (char *) &this->instr, 1);
				strncpy(ret + 1, (char *) &n, sizeof(double));
				break;
			}
			case ArgType::NO_ARG: {
				ret = (char *) realloc(ret, s);
				strncpy(ret, (char *) &this->instr, 1);
				break;
			}
			default: {
				ret = (char *) realloc(ret, s);
				strncpy(ret, (char *) &this->instr, 1);
				break;
			}
		}

		return s;
	}


};


#endif //DLANG_COMMAND_HPP
