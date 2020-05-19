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

		END_LIT_SECTION,
		//END_LIT_STRING,		// just use '\0'...
		END_LIT_MACRO,
		//END_LIT_JSON,			// just use '\0'


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
		CLEAR_STACK,	// semicolon operator
			// no arg
		VAL_EMPTY,
		VAL_TRUE,
		VAL_FALSE,

		MACRO_INVOKE,
			// no arg


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

	Command(OPCode cmd, uint16_t argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, int64_t argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, double argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, std::string argument):
		instr(cmd), arg(std::move(argument)) {}
	explicit Command(OPCode cmd): instr(cmd) {}

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
				return "#### Begin Fault Table ####\n";

				// indented bc must be within macro body
			case OPCode::I64_LIT:
				return "\tI64 " + std::to_string(std::get<int64_t>(this->arg)) + "\n";
			case OPCode::F64_LIT:
				return "\tF64 " + std::to_string(std::get<double>(this->arg))  + "\n";
			case OPCode::DECL_ID:
				return "\tLET_ID " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + "\n";
			case OPCode::USE_ID:
				return "\tUSE_ID " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + "\n";
			case OPCode::USE_LIT:
				return "\tUSE_LIT " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + "\n";
			case OPCode::BUILTIN_OP:
				// switch (std::get<int16_t>(this->arg)) { ... }
				return "\tOP #" + std::to_string(std::get<uint16_t>(this->arg)) + "\n";
			case OPCode::KW_VAL:
				// switch (std::get<int16_t>(this->arg)) { ... }
				return "\tKW_LIT #" + std::to_string(std::get<uint16_t>(this->arg)) + "\n";
			case OPCode::CLEAR_STACK:
				return "\tCLEAR_STACK\n";
			case OPCode::MACRO_INVOKE:
				return "\tINVOKE\n";
			case OPCode::VAL_EMPTY:
				return "\tC_EMPTY";
			case OPCode::VAL_FALSE:
				return "\tC_FALSE";
			case OPCode::VAL_TRUE:
				return "\tC_TRUE";

				// fault table

				// identifier translations
			case OPCode::ID_NAME:
				return "ID_NAME " + std::get<std::string>(this->arg) + " : ";
			case OPCode::ID_ID:
				return "ID_ID " + std::to_string(std::get<int64_t>(this->arg)) + "\n";

			case OPCode::FILE_NAME:
				return "In file: " + std::get<std::string>(this->arg) + "\n";

			case OPCode::DEST_POS:
				return "Compiled Line#" + std::to_string(std::get<int64_t>(this->arg)) + " came from ";
			case OPCode::SRC_POS:
				return "Source Pos#" + std::to_string(std::get<int64_t>(this->arg)) + "\n";
			default:
				return "";
		}
	}

	// check if it goes in literal header
	[[nodiscard]] inline bool is_lit() const noexcept {
		return this->instr < OPCode::END_LIT_SECTION;
	}

	static inline ArgType arg_type(const OPCode instr) {
		switch (instr) {
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

	[[nodiscard]] inline ArgType arg_type() const {
		return Command::arg_type(this->instr);
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
				ret[str.size() + 2] = OPCode::END_LIT_STRING; // hmm
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
