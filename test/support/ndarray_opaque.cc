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
#include <cstdint>
#include "ndarray_opaque.h"

namespace mf { namespace test {

opaque_multi_ndarray_format opaque_frame_format() {
	opaque_multi_ndarray_format frm;
	frm.add_part(make_ndarray_format<std::uint32_t>(7));
	// padding needed between parts
	frm.add_part(make_ndarray_format<std::uint64_t>(7, 2*8)); // padding between part elements
	frm.add_part(make_ndarray_format<std::uint8_t>(3));
	// padding needed at end
	return frm;
}


ndarray_opaque<0> make_opaque_frame(int number) {
	ndarray_opaque<0> arr(make_ndsize(), opaque_frame_format());
	ndarray_view<1, std::uint32_t> part0 = from_opaque<1, std::uint32_t>(extract_part(arr, 0), make_ndsize(7));
	ndarray_view<1, std::uint64_t> part1 = from_opaque<1, std::uint64_t>(extract_part(arr, 1), make_ndsize(7));
	ndarray_view<1, std::uint8_t> part2  = from_opaque<1, std::uint8_t> (extract_part(arr, 2), make_ndsize(3));
	for(std::ptrdiff_t i = 0; i < 7; ++i) part0[i] = number + i;
	for(std::ptrdiff_t i = 0; i < 7; ++i) part1[i] = 2*(number + i) + 1;
	for(std::ptrdiff_t i = 0; i < 3; ++i) part2[i] = 3*(number%100) + 50;
	return arr;
}


int opaque_frame_index(const ndarray_view_opaque<0>& vw, bool verify) {
	ndarray_view<1, std::uint32_t> part0 = from_opaque<1, std::uint32_t>(extract_part(vw, 0), make_ndsize(7));
	int number = part0[0];
	if(verify)
		if(vw != make_opaque_frame(number)) return -1;
	return number;
}


bool compare_opaque_frames(const ndarray_view_opaque<1>& frames, const std::vector<int>& is) {
	if(frames.shape().front() != is.size()) return false;
	for(std::ptrdiff_t i = 0; i < is.size(); ++i) {
		auto expected = make_opaque_frame(is[i]);
		if(frames[i] != expected) return false;
	}
	return true;
}


}}
