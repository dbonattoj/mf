#include <catch.hpp>
#include "ndarray_opaque.h"

namespace mf { namespace test {

ndarray_opaque_frame_format opaque_frame_format() {
	ndarray_opaque_frame_format frm;
	frm.add_part(make_ndarray_format<int>(10));
	frm.add_part(make_ndarray_format<double>(7, 2*sizeof(double)));
	return frm;
}


ndarray_opaque<0> make_opaque_frame(int number) {
	ndarray_opaque<0> arr(make_ndsize(), opaque_frame_format());
	ndarray_view<1, int> part0 = from_opaque<1, int>(extract_part(arr, 0), make_ndsize(10));
	ndarray_view<1, double> part1 = from_opaque<1, double>(extract_part(arr, 1), make_ndsize(7));
	for(std::ptrdiff_t i = 0; i < 10; ++i) part0[i] = number + i;
	for(std::ptrdiff_t i = 0; i < 7; ++i) part1[i] = double(number + i);
	return arr;
}


int opaque_frame_index(const ndarray_view_opaque<0>& vw, bool verify) {
	ndarray_view<1, int> part0 = from_opaque<1, int>(extract_part(vw, 0), make_ndsize(10));
	int number = part0[0];
	if(verify)
		if(vw != make_opaque_frame(number)) return -1;
	return number;
}


///////////////



ndarray_opaque_frame_format opaque_contiguous_frame_format() {
	ndarray_opaque_frame_format frm(make_ndarray_format<int>(10));
	return frm;
}


ndarray_opaque<0> make_contiguous_opaque_frame(int number) {
	ndarray_opaque<0> arr(make_ndsize(), opaque_contiguous_frame_format());
	ndarray_view<1, int> concrete = from_opaque<1, int>(arr.view(), make_ndsize(10));
	for(std::ptrdiff_t i = 0; i < 10; ++i) concrete[i] = number + i;
	return arr;
}


int opaque_contiguous_frame_index(const ndarray_view_opaque<0>& vw, bool verify) {
	ndarray_view<1, int> concrete = from_opaque<1, int>(vw, make_ndsize(10));
	int number = concrete[0];
	if(verify)
		if(vw != make_contiguous_opaque_frame(number)) return -1;
	return number;
}



}}
