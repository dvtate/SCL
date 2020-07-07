//
// Created by tate on 02-05-20.
//

#include "util.hpp"

namespace util {
	const char* term_eff_bold = "\x1B[1m";
	const char* term_eff_red = "\x1B[31m";
	const char* term_eff_reset = "\x1B[0m";

	//
	std::string show_line_pos(std::istream &file, unsigned long long pos, const std::string fname) {
		// rewind
		file.clear();
		file.seekg(0);

		std::string line;
		std::string ret;
		unsigned long long i = 0;
		std::size_t line_num = 1;

		for (; std::getline(file, line); line_num++) {
			if (i + line.length() > pos) {
				unsigned short line_pos = pos - i;
				// ret.reserve(6 + fname.size() + line.size() + line_pos);
				ret += term_eff_bold;
				ret += fname;
				ret += ':';
				ret += std::to_string(line_num);
				ret += ':';
				ret += std::to_string(line_pos);
				ret += term_eff_reset;
				ret += '\n';
				ret += line + '\n';
				for (; line_pos > 0; line_pos--)
					ret += ' ';
				ret += term_eff_red;
				ret += "^\n";
				ret += term_eff_reset;
				return ret;
			} else {
				i += line.length();
			}
		}

		return ret;
	}

}