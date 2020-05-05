//
// Created by tate on 02-05-20.
//

#ifndef DLANG_FRAME_HPP
#define DLANG_FRAME_HPP

#include <vector>
#include <cstdint>

// Execution frame
class Frame {
public:
	static uint64_t _uid;


	std::vector<char> bytecode;





};


#endif //DLANG_FRAME_HPP
