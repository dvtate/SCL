//
// Created by tate on 02-05-20.
//

#include "util.hpp"

namespace util {
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
				ret += fname;
				ret += ':';
				ret += std::to_string(line_num);
				ret += ':';
				ret += std::to_string(line_pos);
				ret += '\n';
				ret += line + '\n';
				for (; line_pos > 0; line_pos--)
					ret += ' ';
				ret += "^\n";

				return ret;
			} else {
				i += line.length();
			}
		}
	}
}