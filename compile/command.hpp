//
// Created by tate on 29-04-20.
//

#ifndef DLANG_COMMAND_HPP
#define DLANG_COMMAND_HPP


class Command {
	enum {

		// these go in lit header
		START_LIT_STRING,
		START_LIT_MACRO,
		START_LIT_JSON,
		END_LIT_STRING,
		END_LIT_MACRO,
		END_LIT_JSON,

		// these can go in macro bodies
		I64_LIT,
		F64_LIT,
		DECL_ID,
		USE_ID,
		OP_EQ,
		OP_PLUS,
		OP_MINUS,
		OP_STAR,
		OP_SLASH,


	};

};


#endif //DLANG_COMMAND_HPP
