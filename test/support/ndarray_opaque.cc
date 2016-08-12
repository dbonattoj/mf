#include <catch.hpp>
#include <cstdint>
#include "ndarray_opaque.h"

namespace mf { namespace test {

ndarray_opaque_frame_format opaque_frame_format() {
	ndarray_opaque_frame_format frm;
	frm.add_part(make_ndarray_format<std::uint32_t>(7));
	// padding needed between parts
	frm.add_part(make_ndarray_format<std::uint64_t>(7, 2*8)); // padding between part elements
	frm.add_part(make_ndarray_format<std::uint8_t>(3));
	// padding needed at end
	return frm;
}


ndarray_opaque<0> make_opaque_frame(int number) {
	ndarray_opaque<0> arr(make_ndsize(), opaque_frame_format());
	ndarray_view<1, std::uint32_t> part0 = from_opaque<1, std::uint32_t>(extract_part(arr, 0), make_ndsize(10));
	ndarray_view<1, std::uint64_t> part1 = from_opaque<1, std::uint64_t>(extract_part(arr, 1), make_ndsize(7));
	ndarray_view<1, std::uint8_t> part2 = from_opaque<1, std::uint8_t>(extract_part(arr, 2), make_ndsize(3));
	for(std::ptrdiff_t i = 0; i < 10; ++i) part0[i] = number + i;
	for(std::ptrdiff_t i = 0; i < 7; ++i) part1[i] = 2*(number + i) + 1;
	for(std::ptrdiff_t i = 0; i < 3; ++i) part2[i] = 3*(number%100) + 50;
	return arr;
}


int opaque_frame_index(const ndarray_view_opaque<0>& vw, bool verify) {
	ndarray_view<1, std::uint32_t> part0 = from_opaque<1, std::uint32_t>(extract_part(vw, 0), make_ndsize(10));
	int number = part0[0];
	if(verify)
		if(vw != make_opaque_frame(number)) return -1;
	return number;
}



}}
