//
// Created by tate on 17/06/2021.
//

#ifndef SCL_PARSE_HPP
#define SCL_PARSE_HPP

#include <optional>

#include "../../vm/vm.hpp"
#include "../../vm/error.hpp"

// Exported native function
static NativeFunction* parse_nfn;

class JSONParseFn : public NativeFunction {
	// Recursive descent parser roughly adhering to
	// https://www.json.org/json-en.html <- general idea
	// https://datatracker.ietf.org/doc/html/rfc8259 <- more details

	/// Thrown on invalid JSON input
	class ParseError : public std::exception {
	public:
		// Character index
		size_t pos;

		// Why it's bad
		std::string message;

		ParseError(size_t pos, std::string message):
			pos(pos), message(std::move(message))
		{}
	};

	/// Skip whitespaces
	static void ws(const std::string& s, size_t& i) {
		while (i < s.size() && isspace(s[i]))
			if (s[i] != ' ' && s[i] != '\t' && s[i] != '\n' && s[i] != '\r')
				return;
			else
				i++;
	}

	/// Parse keyword
	static std::optional<Value> keyword(const std::string& s, size_t& i) {
		const char* sptr = s.c_str() + i;
		if (strncmp(sptr, "true", 4) == 0)
			return Value(true);
		if (strncmp(sptr, "false", 5) == 0)
			return Value(false);
		if (strncmp(sptr, "null", 4) == 0)
			return Value();
		return std::nullopt;
	}

	/// Parse string literal
	static std::optional<Value> str(const std::string& s, size_t& i) {
		// Read start
		if (s[i] != '"')
			return std::nullopt;
		i++;
		std::string ret;

		while (i < s.size()) {
			// End string
			if (s[i] == '"') {
				i++;
				return Value(ret);
			}

			// Control code
			if (s[i] == '\\') {
				i++;
				if (i >= s.size())
					throw ParseError(i - 1, "Unterminated string literal after \\");
				switch (s[i]) {
					case '"':
						ret += '"';
						i++;
						break;
					case '\\':
						ret += '\\';
						i++;
						break;
					case 'b':
						ret += '\b';
						i++;
						break;
					case 'f':
						ret += '\f';
						i++;
						break;
					case 'n':
						ret += '\n';
						i++;
						break;
					case 'r':
						ret += '\r';
						i++;
						break;
					case 't':
						ret += '\t';
						i++;
						break;

					// \u followed by 4 hex digits
					case 'u': {
						const auto hex_seq = s.substr(i + 1, 4);
						char* p;
						const auto hex_val = (int16_t) strtol(hex_seq.c_str(), &p, 16);
						if (*p != '\0')
							throw ParseError(i, "Invalid \\uXXXX hex code - " + hex_seq);
						if (hex_val >> 8)
							ret += (char) (hex_val >> 8);
						ret += (char) (hex_val & 0b1111'1111);
						i += 5;
						break;
					}

					// Invalid escape sequence
					default:
						throw ParseError(i, std::string("Unexpected token ") + s[i]);
				}
				continue;
			}

			// TODO handle invalid characters

			// Normal character
			ret += s[i++];
		}

		// no closing "
		throw ParseError(i, "Unterminated string literal");
	}

	/// Parse number literal
	static std::optional<Value> num(const std::string& s, size_t& i) {
		// Read minus
		auto start = i;
		bool minus = false;
		if (s[i] == '-') {
			minus = true;
			i++;
			if (i >= s.size())
				throw ParseError(i, "unexpected end of input");
		}

		// Read int part (only part that's non-optional)
		if (s[i] == '0') {
			i++;
		} else if (isdigit(s[i])) {
			i++;
			while (i < s.size() && isdigit(s[i]))
				i++;
		} else if (!minus) {
			return std::nullopt;
		} else {
			throw ParseError(i, "unexpected token -");
		}

		// Read decimal part
		bool decimal = false;
		if (s[i] == '.') {
			decimal = true;
			i++;
			while (i < s.size() && isdigit(s[i]))
				i++;
		}

		// Read exponent part
		bool exponent = false;
		if (s[i] == 'E' || s[i] == 'e') {
			exponent = true;
			i++;
			if (s[i] == '+' || s[i] == '-')
				i++;
			while (i < s.size() && isdigit(s[i]))
				i++;
		}

		// Return an int
		if (!decimal && !exponent) {
			try {
				size_t end_idx;
				const auto ret = std::stol(s.substr(start, i - start), &end_idx);
				if (end_idx + start != i)
					std::cout << "stol() end not aligined " << i << ' ' << end_idx << std::endl;
				return Value(ret);
			} catch (const std::out_of_range&) {
				// When too big for an int return float
				// continue
			}
		}

		// Return a float
		size_t end_idx;
		const auto ret = std::stod(s.substr(start, i - start), &end_idx);
		if (end_idx + start != i)
			std::cout <<"stod() end not aligined " <<i <<' ' <<end_idx <<std::endl;
		return Value(ret);
	}

	/// Parse Array
	static std::optional<Value> array(const std::string& s, size_t& i, Frame& f) {
		// Arrays start with '['
		if (s[i] != '[')
			return std::nullopt;
		i++;

		auto* ret = f.gc_make<ValueTypes::list_t>();

		// Empty list
		ws(s, i);
		if (s[i] == ']') {
			i++;
			return Value(ret);
		}

		// Read elements of list
		while (i < s.size()) {
			// Push element into list
			const auto e = value(s, i, f);
			if (!e)
				throw ParseError(i, std::string("unexpected token ") + s[i]);
			else
				ret->emplace_back(*e);

			// EOF
			if (i >= s.size())
				throw ParseError(i, "unexpected end of input");

			// End of list or next item
			if (s[i] == ']') {
				i++;
				return Value(ret);
			}
			if (s[i] != ',')
				throw ParseError(i, "expected comma or right square bracket");
			i++;
		}
		throw ParseError(i, "unclosed array");
	}

	/// Read Object
	static std::optional<Value> object(const std::string& s, size_t& i, Frame& f) {
		// Objects start with '{'
		if (s[i] != '{')
			return std::nullopt;
		i++;

		auto* ret = f.gc_make<ValueTypes::obj_t>();

		// Empty object
		ws(s, i);
		if (i >= s.size())
			throw ParseError(i, "Unexpected end of input");
		if (s[i] == '}') {
			i++;
			return Value(ret);
		}

		// Process entries
		while (i < s.size()) {
			// Get key
			ws(s, i);
			auto k = str(s, i);
			if (!k)
				throw ParseError(i, "Expected string");

			// Get value
			ws(s, i);
			if (s[i] != ':')
				throw ParseError(i, "Expected colon");
			i++;
			auto v = value(s, i, f);
			if (!v)
				throw ParseError(i, std::string("unexpected token ") + s[i]);

			// Push pair
			ret->emplace(k->get<ValueTypes::str_t>(), *v);

			// Next
			if (s[i] == '}') {
				i++;
				return Value(ret);
			}
			if (s[i] != ',')
				throw ParseError(i, "expected comma or closing curly brace");
			i++;
		}
		throw ParseError(i, "unclosed object");
	}

	/// Parse JSON value
	static std::optional<Value> value(const std::string& s, size_t& i, Frame& f) {
		ws(s, i);
		auto ret = str(s, i);
		if (!ret) ret = array(s, i, f);
		if (!ret) ret = object(s, i, f);
		if (!ret) ret = num(s, i);
		if (!ret) ret = keyword(s, i);
		ws(s, i);
		return ret;
	}

public:
	void operator()(Frame& f) override {
		// Complain if arg not a string
		const auto& arg = f.eval_stack.back();
		if (arg.type() != ValueTypes::VType::STR) {
			f.rt->running->throw_error(gen_error_object(
					"TypeError",
					"JSON parse expected a string, not a " + arg.type_name(),
					f));
			return;
		}

		// Parse value
		// Translate errors
		// Throw if any remaining content
		size_t i = 0;
		const auto& str = arg.get<ValueTypes::str_t>();
		try {
			auto ret = value(str, i, f);
			if (!ret)
				throw ParseError(0, "invalid JSON syntax");
			f.eval_stack.back() = *ret;
		} catch (const ParseError& e) {
			f.rt->running->throw_error(gen_error_object(
					"JSON Syntax Error",
					e.message + " at index " + std::to_string(e.pos),
					f));
			return;
		}
	}

	void mark() override {}
};

#endif //SCL_PARSE_HPP
