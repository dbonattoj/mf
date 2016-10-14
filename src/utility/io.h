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

#ifndef MF_UTILITY_IO_H_
#define MF_UTILITY_IO_H_

#include <iosfwd>
#include <limits>
#include <string>
#include "../common.h"

namespace mf {

enum class line_delimitor { unknown, LF, CR, CRLF };

const extern line_delimitor default_line_delimitor;
const extern bool host_has_iec559_float;
const extern bool host_is_little_endian;

/// Detects line delimitor used in given file.
/** Reads up to max_offset characters into file until line delimitor is found. Reads from current stream position,
 ** and rewinds to that starting position afterwards. Throws exception if no line ending detected. */
line_delimitor detect_line_delimitor(std::istream&, std::size_t max_offset = 512);

void read_line(std::istream&, std::string&, line_delimitor = default_line_delimitor);
void skip_line(std::istream&, line_delimitor = default_line_delimitor);
void write_line(std::ostream&, const std::string&, line_delimitor = default_line_delimitor);
void end_line(std::ostream&, line_delimitor = default_line_delimitor);

void flip_endianness(byte* data, std::size_t sz);

template<typename T> void flip_endianness(T& t) {
	flip_endianness(reinterpret_cast<byte*>(&t), sizeof(T));
}

std::streamoff file_size(const std::string&);
std::streamoff file_size(std::ifstream&);

}

#endif
