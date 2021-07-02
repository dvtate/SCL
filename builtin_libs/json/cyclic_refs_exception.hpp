//
// Created by tate on 01/07/2021.
//

#ifndef SCL_CYCLIC_REFS_EXCEPTION_HPP
#define SCL_CYCLIC_REFS_EXCEPTION_HPP

#include <string>

// Error for cyclic references
class CyclicRefsEx : public std::exception {
public:
	std::string trace {"Cyclic Reference: "};
	CyclicRefsEx() noexcept {}
	CyclicRefsEx(CyclicRefsEx& other) noexcept : trace(other.trace) {}

	[[nodiscard]] const char* what() const noexcept override {
		return this->trace.c_str();
	}

	/// Add to trace
	void push(const std::string& frame) {
		this->trace += "\n" + frame;
	}
};

#endif //SCL_CYCLIC_REFS_EXCEPTION_HPP
