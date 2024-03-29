//
// Created by tate on 02-05-20.
//

#ifndef SCL_UTIL_HPP
#define SCL_UTIL_HPP

#include <string>
#include <istream>
#include <iostream>
#include <fstream>

namespace util {
	// Escape sequences
	extern const char* term_eff_bold;
	extern const char* term_eff_red;
	extern const char* term_eff_reset;

	/// show full line at position pos in file
	std::string show_line_pos(std::istream& file, unsigned long long pos, std::string fname);

	inline std::string show_line_pos(const std::string& fname, const unsigned long long pos)
	{
		std::ifstream f(fname, std::ifstream::in);
		return show_line_pos(f, pos, fname);
	}

	std::pair<long, long> pos_to_line_offset(std::istream& file, const unsigned long long pos);
}

#endif //SCL_UTIL_HPP
