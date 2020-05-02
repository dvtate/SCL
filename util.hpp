//
// Created by tate on 02-05-20.
//

#ifndef DLANG_UTIL_HPP
#define DLANG_UTIL_HPP

#include <string>
#include <istream>
#include <iostream>
#include <fstream>

namespace util {

	// show full line at position pos in file
	std::string show_line_pos(std::istream& file, unsigned long long pos, std::string fname);

	inline std::string show_line_pos(const std::string& fname, const unsigned long long pos)
	{
		std::ifstream f(fname, std::ifstream::in);
		return show_line_pos(f, pos, fname);
	}
}



#endif //DLANG_UTIL_HPP
