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

#include <catch.hpp>
#include <algorithm>
#include "../src/os/memory.h"
#include "../src/common.h"

using namespace mf;


TEST_CASE("ring_allocator", "[ring_allocator]") {
	raw_ring_allocator allocator;
	std::size_t n = raw_round_up_to_fit_system_page_size(100);
	
	// allocate
	void* buf_raw = allocator.raw_allocate(n);
	REQUIRE(buf_raw != nullptr);
	byte* buf = static_cast<byte*>(buf_raw);
	
	// write in base, read in mirror
	buf[7] = byte(123);
	REQUIRE(buf[7] == byte(123));
	REQUIRE(buf[n + 7] == byte(123));
	
	// write in mirror, read in base
	buf[n + 3] = byte(456);
	REQUIRE(buf[n + 3] == byte(456));
	REQUIRE(buf[3] == byte(456));
	
	// write segment, warp-around
	std::vector<byte> data;
	for(byte i = 0; i < byte(100); ++i) data.push_back(i);
	std::ptrdiff_t beg = n - 50;
	std::copy(data.cbegin(), data.cend(), buf + beg);
	REQUIRE(std::equal(data.cbegin(), data.cend(), buf + beg));
	REQUIRE(std::equal(data.cbegin() + 50, data.cend(), buf));
	
	// deallocate
	allocator.raw_deallocate(buf_raw, n);
}
