//
// Created by tate on 29-04-20.
//

#ifndef SCL_COMMAND_HPP
#define SCL_COMMAND_HPP

#include <cinttypes>
#include <cstdlib>
#include <variant>
#include <cstring>
#include <string>

class Command {
public:
	/// mnemonic instruction
	enum OPCode : unsigned char {
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
		SET_ID,
			// arg: int64, idid
		USE_ID,			// get value at identifier
			// arg: int64, idid
		USE_LIT,		// push literal onto stack
			// arg: int64, lit id
		BUILTIN_OP,		// operate on stack
			// arg: int16, builtin operator id
		KW_VAL, 		// builtin keyword-literal (remove?)
			// arg: int16
		CLEAR_STACK,	// semicolon operator
			// no arg
		MK_LIST,		// constructs a list from values on stack
			// arg: int32, number of items to pull from stack

		USE_MEM_L,		// gets member from an object (using literal number for mem name)
			// arg: lit id int64
			// stack args: <obj> USE_MEM_L(lit_num)
		SET_MEM_L,		// sets object member (using literal number for mem name)
			// arg: lit id int64
			// stack args: <value> <obj> SET_MEM_L(lit_num)
		MK_OBJ,			// construct object from values on stack
			// arg: number of items to pull from stack

		VAL_EMPTY,
		VAL_TRUE,
		VAL_FALSE,
		VAL_CATCH,	// pushes function used to set error handler for current macro

		INVOKE,			// <i> <fn> INVOKE
		USE_INDEX,		// <list> <idx> INVOKE
		SET_INDEX,		// <list> <index> <value> SET_INDEX
			// args come from stack only

		//////////
		// begin fault table
		//////////

		// Mutilated identifiers
		ID_NAME,		// user-defined identifier name
			// arg: str
		ID_ID, 			// compiled identifier number
			// arg: int64


		// invoke lhs for stack traces
		// TODO maybe it would be good to put these in lit header or sth so that there isn't as much string duplication?
		INVOKE_REPR,	// The thing being invoked
			//  arg: str
		INVOKE_POS,		// Where in the program is the invocation taking place
			//	arg: int64

		//
		FILE_NAME,		// file path
			// arg: str
		DEST_POS,		// location in compiled file
			// arg: int64
		SRC_POS,		// original file position
			// arg: int64
	} instr;

	std::variant<uint16_t, int32_t, int64_t, double, std::string> arg;

	enum ArgType {
		INT64, INT16, FLOAT, STRING, NO_ARG, INT32
	};

	Command(OPCode cmd, uint16_t argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, int64_t argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, int32_t argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, double argument):
		instr(cmd), arg(argument) {}
	Command(OPCode cmd, std::string argument):
		instr(cmd), arg(std::move(argument)) {}
	explicit Command(OPCode cmd): instr(cmd) {}


	static inline const char* cmd_name(OPCode c) {
		switch (c) {
			case END_LIT_STRING:	return "END_LIT_STRING";
			case START_LIT_STRING:	return "START_LIT_STRING";
			case START_LIT_MACRO:	return "START_LIT_MACRO";
			case START_LIT_JSON:	return "START_LIT_JSON";
			case END_LIT_SECTION:	return "END_LIT_SECTION";
			case END_LIT_MACRO:		return "END_LIT_MACRO";
			case I64_LIT:	return "I64_LIT";
			case F64_LIT:	return "F64_LIT";
			case DECL_ID:	return "DECL_ID";
			case SET_ID:	return "SET_ID";
			case USE_ID:	return "USE_ID";
			case USE_LIT:	return "USE_LIT";
			case BUILTIN_OP:return "BUILTIN_OP";
			case KW_VAL:	return "KW_VAL";
			case CLEAR_STACK:	return "CLEAR_STACK";
			case MK_LIST: 	return "MK_LIST";
			case VAL_EMPTY: return "VAL_EMPTY";
			case VAL_TRUE:	return "VAL_TRUE";
			case VAL_FALSE:	return "VAL_FALSE";
			case VAL_CATCH: return "VAL_CATCH";
			case INVOKE:	return "INVOKE";
			case USE_INDEX: return "USE_INDEX";
			case SET_INDEX: return "SET_INDEX";
			case ID_NAME:	return "ID_NAME";
			case ID_ID:		return "ID_ID";
			case FILE_NAME:	return "FILE_NAME";
			case DEST_POS:	return "DEST_POS";
			case SRC_POS:	return "SRC_POS";
			case USE_MEM_L: return "USE_MEM_L";
			case SET_MEM_L: return "SET_MEM_L";
			case MK_OBJ:	return "MK_OBJ";
			case INVOKE_REPR:	return "INVOKE_REPR";
			case INVOKE_POS:	return "INVOKE_POS";
			default:		return "UNKNOWN";
		}
	}

	/// used for generating bytecode text format for debugging
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
				return "\tDECL_ID " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + "\n";
			case OPCode::SET_ID:
				return "\tSET_ID " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + "\n";
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
			case OPCode::INVOKE:
				return "\tINVOKE\n";
			case OPCode::USE_INDEX:
				return "\tUSE_INDEX\n";
			case OPCode::VAL_EMPTY:
				return "\tC_EMPTY\n";
			case OPCode::VAL_FALSE:
				return "\tC_FALSE\n";
			case OPCode::VAL_TRUE:
				return "\tC_TRUE\n";
			case OPCode::VAL_CATCH:
				return "\tVAL_CATCH\n";
			case OPCode::MK_LIST:
				return "\tMAKE_LIST(" + std::to_string(std::get<int32_t>(this->arg)) + ")\n";

			case OPCode::MK_OBJ:
				return "\tMK_OBJ(" + std::to_string(std::get<int32_t>(this->arg)) + ")\n";
			case OPCode::USE_MEM_L:
				return "\tUSE_MEM_Lit[" + std::to_string(std::get<int64_t>(this->arg)) + "]\n";
			case OPCode::SET_MEM_L:
				return "\tSET_MEM_Lit[" + std::to_string(std::get<int64_t>(this->arg)) + "]\n";
			case OPCode::SET_INDEX:
				return "\tSET_INDEX\n";

				// fault table

				// identifier translations
			case OPCode::ID_NAME:
				return "ID_NAME " + std::get<std::string>(this->arg) + " : ";
			case OPCode::ID_ID:
				return "ID_ID " + std::to_string(std::get<int64_t>(this->arg)) + "\n";

			case INVOKE_POS:
				return "INVOKE_POS " + std::to_string((uint64_t) std::get<int64_t>(this->arg)) + " : ";
			case INVOKE_REPR:
				return "INVOKE_REPR " + std::get<std::string>(this->arg) + "\n";

			case OPCode::FILE_NAME:
				return "In file: " + std::get<std::string>(this->arg) + "\n";

			case OPCode::DEST_POS:
				return "Compiled Line#" + std::to_string(std::get<int64_t>(this->arg)) + " came from ";
			case OPCode::SRC_POS:
				return "Source Pos#" + std::to_string(std::get<int64_t>(this->arg)) + "\n";
			default: return "???\n";
		}
	}

	/// check if it goes in literal header
	[[nodiscard]] inline bool is_lit() const noexcept {
		return this->instr < OPCode::END_LIT_SECTION;
	}

	static inline ArgType arg_type(const OPCode instr) {
		switch (instr) {
			case OPCode::F64_LIT:
				return ArgType::FLOAT;

			// TODO: these can all be converted to int32_t
			case OPCode::I64_LIT: case OPCode::DECL_ID: case OPCode::SET_ID: case OPCode::USE_ID:
			case OPCode::USE_LIT: case OPCode::ID_ID: case OPCode::SRC_POS: case OPCode::DEST_POS:
			case OPCode::USE_MEM_L: case OPCode::SET_MEM_L: case OPCode::INVOKE_POS:
				return ArgType::INT64;

			case OPCode::BUILTIN_OP: case OPCode::KW_VAL:
				return ArgType::INT16;
			case OPCode::START_LIT_STRING: case OPCode::START_LIT_JSON: case OPCode::ID_NAME: case OPCode::FILE_NAME:
			case OPCode::INVOKE_REPR:
				return ArgType::STRING;
			case OPCode::MK_LIST: case OPCode::MK_OBJ:
				return ArgType::INT32;

			default:
				return ArgType::NO_ARG;
		}
	}

	[[nodiscard]] inline ArgType arg_type() const {
		return Command::arg_type(this->instr);
	}

	/**
	 * Compiles command to a bytecode instruction
	 * @param ret referenced buffer to put the compiled instruction
	 * @returns number of chars read
	 */
	std::size_t compile(char*& ret) {
		ArgType t = arg_type();
		size_t s = 1;
		switch (t) {
			case ArgType::STRING: {
				auto &str = std::get<std::string>(this->arg);
				s += str.size() + 1;
				ret = (char *) realloc(ret, s);
				ret[0] = this->instr;
				strcpy(ret + 1, str.c_str()); // also copies the '\0'
				ret[str.size() + 1] = OPCode::END_LIT_STRING; // hmm
				break;
			}
			case ArgType::INT16: {
				s += sizeof(uint16_t);
				ret = (char *) realloc(ret, s);
				uint16_t n = std::get<uint16_t>(this->arg);
				ret[0] = this->instr;
				strncpy(ret + 1, (char *) &n, sizeof(uint16_t));
				break;
			}
			case ArgType::INT32: {
				s += sizeof(int32_t);
				ret = (char *) realloc(ret, s);
				int32_t n = std::get<int32_t>(this->arg);
				ret[0] = this->instr;
				strncpy(ret + 1, (char *) &n, sizeof(int32_t));
				break;
			}
			case ArgType::INT64: {
				s += sizeof(int64_t);
				ret = (char *) realloc(ret, s);
				int64_t n = std::get<int64_t>(this->arg);
				ret[0] = this->instr;
				strncpy(ret + 1, (char *) &n, sizeof(int64_t));
				break;
			}
			case ArgType::FLOAT: {
				s += sizeof(double);
				ret = (char *) realloc(ret, s);
				double n = std::get<double>(this->arg);
				ret[0] = this->instr;
				strncpy(ret + 1, (char *) &n, sizeof(double));
				break;
			}
			case ArgType::NO_ARG:
			default: {
				ret = (char *) realloc(ret, s);
				ret[0] = this->instr;
				break;
			}
		}

		return s;
	}

	/**
	 * Verifies that arg holds correct type
	 *
	 * @returns true if valid false if something's wrong
	 */
	[[nodiscard]] bool check_arg() const {
		const auto ind = this->arg.index();
		switch (this->arg_type()) {
			case ArgType::INT16:	return ind == 0;
			case ArgType::INT32:	return ind == 1;
			case ArgType::INT64:	return ind == 2;
			case ArgType::FLOAT:	return ind == 3;
			case ArgType::STRING:	return ind == 4;
			default:				return true;
		}
	}
};


#endif //SCL_COMMAND_HPP
