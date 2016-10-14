/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "io.h"

#include <istream>
#include <ostream>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <type_traits>
#include <cctype>

namespace mf {

namespace {
	bool check_host_little_endian_() {
		union {
			std::uint16_t s;
			std::uint8_t c[2];
		} d;
		d.s = 1;
		return d.c[0] == 1;
	}
}


const line_delimitor default_line_delimitor = line_delimitor::LF;

const bool host_has_iec559_float =
	std::numeric_limits<float>::is_iec559 && std::numeric_limits<double>::is_iec559;

const bool host_is_little_endian = check_host_little_endian_();



line_delimitor detect_line_delimitor(std::istream& str, std::size_t max_offset) {
	std::size_t counter = 0;
	line_delimitor ld = line_delimitor::unknown;
	auto old_position = str.tellg();
	while(ld == line_delimitor::unknown) {
		if(++counter > max_offset) throw std::runtime_error("could not detect file line ending");
		char c;
		str.get(c);
		if(c == '\n') ld = line_delimitor::LF;
		else if(c == '\r') {
			if(str.peek() == '\n') ld = line_delimitor::CRLF;
			else ld = line_delimitor::CR;
		}
	}
	str.seekg(old_position);
	return ld;
}

void read_line(std::istream& str, std::string& line, line_delimitor ld) {
	if(ld == line_delimitor::unknown) throw std::invalid_argument("line delimitor not specified");
	
	if(ld == line_delimitor::CRLF) {
		std::getline(str, line, '\r');
		while(str.peek() != '\n') {
			std::string part_line;
			std::getline(str, part_line, '\r');
			line.append(1, '\r').append(part_line);
		}
		str.get();
	} else {
		std::getline(str, line, (ld == line_delimitor::LF ? '\n' : '\r'));
	}
}

void skip_line(std::istream& str, line_delimitor ld) {
	if(ld == line_delimitor::unknown) throw std::invalid_argument("line delimitor not specified");
	const auto max_count = std::numeric_limits<std::streamsize>::max();

	if(ld == line_delimitor::CRLF) {
		str.ignore(max_count, '\r');
		while(str.peek() != '\n') str.ignore(max_count, '\r');
		str.ignore();
	} else {
		str.ignore(max_count, (ld == line_delimitor::LF ? '\n' : '\r'));
	}
}


void write_line(std::ostream& str, const std::string& line, line_delimitor ld) {
	str << line;
	end_line(str, ld);
}


void end_line(std::ostream& str, line_delimitor ld) {
	switch(ld) {
		case line_delimitor::LF: str << '\n'; break;
		case line_delimitor::CR: str << '\r'; break;
		case line_delimitor::CRLF: str << "\r\n"; break;
		default: throw std::invalid_argument("line delimitor not specified");
	}
}




void flip_endianness(byte* data, std::size_t sz) {
	if(sz < 2) return;
	std::ptrdiff_t i = sz/2 - 1;
	std::ptrdiff_t o = sz - i - 1;
	while(i >= 0) std::swap(data[i--], data[o++]);
}


std::streamoff file_size(const std::string& filename) {
	std::ifstream stream(filename, std::ifstream::ate | std::ifstream::binary);
	return stream.tellg();
}


std::streamoff file_size(std::ifstream& stream) {
	auto pos = stream.tellg();
	stream.seekg(0, std::ifstream::end);
	auto size = stream.tellg();
	stream.seekg(pos);
	return size;
}

}
