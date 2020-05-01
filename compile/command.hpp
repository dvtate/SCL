//
// Created by tate on 29-04-20.
//

#ifndef DLANG_COMMAND_HPP
#define DLANG_COMMAND_HPP

#include <inttypes.h>

class Command {

	// mnemonic instruction
	enum OPCode : uint8_t {

		// these go in lit header
		START_LIT_STRING,
		START_LIT_MACRO,
		START_LIT_JSON,
		//END_LIT_STRING,		// just use '\0'...
		END_LIT_MACRO,
		END_LIT_JSON,

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

	} instr;


	union Arg {

	} arg;

	enum ArgType {

	} arg_type;
};


#endif //DLANG_COMMAND_HPP
