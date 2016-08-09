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
#include <mf/ndarray/generic/ndarray_view_generic.h>
#include <mf/ndarray/generic/ndarray_timed_view_generic.h>
#include <mf/ndarray/generic/ndarray_generic.h>

using namespace mf;

TEST_CASE("ndarray frame_format", "[nd][ndarray_view_generic][frame_format]") {
	SECTION("default") {
		frame_format frm;
		REQUIRE_FALSE(frm.is_defined());
		
		frame_array_format afrm;
		REQUIRE_FALSE(afrm.is_defined());
	}

	SECTION("one array") {
		constexpr std::size_t n = 100;
		frame_format frm1 = make_frame_array_format<int>(n);
		REQUIRE(frm1.is_defined());
		REQUIRE(frm1.frame_size() == n * sizeof(int));
		REQUIRE(frm1.frame_alignment_requirement() == alignof(int));
		REQUIRE(frm1.arrays_count() == 1);
		REQUIRE(frm1.array_at(0).is_defined());
		REQUIRE(frm1.array_at(0).frame_size() == n * sizeof(int));
		REQUIRE(frm1.array_at(0).frame_alignment_requirement() == alignof(int));
		REQUIRE(frm1.array_at(0).elem_count() == n);
		REQUIRE(frm1.array_at(0).elem_stride() == sizeof(int));
		REQUIRE(frm1.array_at(0).offset() == 0);
		REQUIRE(frm1.array_at(0).elem_size() == sizeof(int));
		REQUIRE(frm1.array_at(0).elem_alignment() == alignof(int));
		REQUIRE(frm1.array_at(0).elem_padding() == 0);
		
		std::size_t s = 3*sizeof(int);
		std::size_t off = 4*sizeof(int);
		frame_format frm2 = make_frame_array_format<int>(n, s, 4*sizeof(int));
		REQUIRE(frm2.is_defined());
		REQUIRE(frm2.frame_size() == off + n*s);
		REQUIRE(frm2.frame_alignment_requirement() == alignof(int));
		REQUIRE(frm2.arrays_count() == 1);
		REQUIRE(frm2.array_at(0).is_defined());
		REQUIRE(frm2.array_at(0).frame_size() == off + n*s);
		REQUIRE(frm2.array_at(0).frame_alignment_requirement() == alignof(int));
		REQUIRE(frm2.array_at(0).elem_count() == n);
		REQUIRE(frm2.array_at(0).elem_stride() == s);
		REQUIRE(frm2.array_at(0).offset() == 4*sizeof(int));
		REQUIRE(frm2.array_at(0).elem_size() == sizeof(int));
		REQUIRE(frm2.array_at(0).elem_alignment() == alignof(int));
		REQUIRE(frm2.array_at(0).elem_padding() == 2*sizeof(int));

		REQUIRE_THROWS( make_frame_array_format<int>(100, alignof(int)/2) );
		REQUIRE_THROWS( make_frame_array_format<int>(100, s, alignof(int)/2) );
	}
	
	SECTION("two arrays") {
		constexpr std::size_t n1 = 33, n2 = 40, s2 = 3*sizeof(double);
		frame_array_format arr1 = make_frame_array_format<int>(n1);
		frame_array_format arr2 = make_frame_array_format<double>(n2, s2);
		frame_format frm;
		frm.place_next_array(arr1);
		frm.place_next_array(arr2);

		REQUIRE(frm.is_defined());
		REQUIRE(frm.frame_size() >= arr1.frame_size() + arr2.frame_size());
		REQUIRE(frm.frame_alignment_requirement() == std::max(alignof(int), alignof(double)));
		REQUIRE(frm.arrays_count() == 2);

		REQUIRE(frm.array_at(0).is_defined());
		REQUIRE(frm.array_at(0).frame_size() == n1 * sizeof(int));
		REQUIRE(frm.array_at(0).frame_alignment_requirement() == alignof(int));
		REQUIRE(frm.array_at(0).elem_count() == n1);
		REQUIRE(frm.array_at(0).elem_stride() == sizeof(int));
		REQUIRE(frm.array_at(0).offset() == 0);
		REQUIRE(frm.array_at(0).elem_size() == sizeof(int));
		REQUIRE(frm.array_at(0).elem_alignment() == alignof(int));
		REQUIRE(frm.array_at(0).elem_padding() == 0);

		REQUIRE(frm.array_at(1).is_defined());
		REQUIRE(frm.array_at(1).frame_size() == frm.array_at(1).offset() + n2 * s2);
		REQUIRE(frm.array_at(1).frame_alignment_requirement() == alignof(double));
		REQUIRE(frm.array_at(1).elem_count() == n2);
		REQUIRE(frm.array_at(1).elem_stride() == s2);
		REQUIRE(frm.array_at(1).offset() >= frm.array_at(0).frame_size());
		REQUIRE(frm.array_at(1).offset() % frm.array_at(1).elem_alignment() == 0);
		REQUIRE(frm.array_at(1).elem_size() == sizeof(double));
		REQUIRE(frm.array_at(1).elem_alignment() == alignof(double));
		REQUIRE(frm.array_at(1).elem_padding() == s2 - sizeof(double));
	}
	
	SECTION("from ndarray_view") {
		constexpr std::size_t pad = sizeof(int);
		constexpr std::size_t len = 3 * 4 * 4;
		std::vector<int> raw(2 * len);
		for(int i = 0; i < 2*len; i += 2) raw[i] = i;
		auto shp = make_ndsize(3, 4, 4);
		auto str = ndarray_view<3, int>::default_strides(shp, pad);
		ndarray_view<3, int> vw(raw.data(), shp, str);
		
		SECTION("full") {
			frame_array_format afrm = format(vw);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == (3*4*4) * str.back());
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.elem_count() == 3*4*4);
			REQUIRE(afrm.elem_stride() == str.back());
			REQUIRE(afrm.offset() == 0);
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == pad);
		}

		SECTION("tail 2") {
			frame_array_format afrm = tail_format<2>(vw);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == (4*4) * str.back());
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.elem_count() == 4*4);
			REQUIRE(afrm.elem_stride() == str.back());
			REQUIRE(afrm.offset() == 0);
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == pad);
		}
		
		SECTION("tail 3") {
			frame_array_format afrm = tail_format<1>(vw);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == 4 * str.back());
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.elem_count() == 4);
			REQUIRE(afrm.elem_stride() == str.back());
			REQUIRE(afrm.offset() == 0);
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == pad);
		}
	}
}


TEST_CASE("ndarray_view <--> ndarray_view_generic", "[nd][ndarray_view_generic][frame_format]") {
	constexpr std::size_t pad = sizeof(int);
	constexpr std::size_t len = 3 * 4 * 4;
	std::vector<int> raw(2 * len);
	for(int i = 0; i < 2*len; i += 2) raw[i] = i;
	auto shp = make_ndsize(3, 4, 4);
	auto str = ndarray_view<3, int>::default_strides(shp, pad);
	
	SECTION("default stride, with padding") {
		ndarray_view<3, int> arr(raw.data(), shp, str);
		std::size_t s = arr.strides().back();
				
		SECTION("generic dimension 0") {
			ndarray_view_generic<0> gen0 = to_generic<0>(arr);
			REQUIRE(gen0.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen0.shape() == make_ndsize());
			REQUIRE(gen0.strides() == make_ndptrdiff());
			
			REQUIRE(gen0.format().is_defined());
			REQUIRE(gen0.format().frame_size() == shp.front() * str.front());
			REQUIRE(gen0.format().frame_alignment_requirement() == alignof(int));
			REQUIRE(gen0.format().arrays_count() == 1);
			REQUIRE(gen0.format().array_at(0).is_defined());
			REQUIRE(gen0.format().array_at(0).frame_size() == shp.front() * str.front());
			REQUIRE(gen0.format().array_at(0).frame_alignment_requirement() == alignof(int));
			REQUIRE(gen0.format().array_at(0).elem_count() == shp.product());
			REQUIRE(gen0.format().array_at(0).elem_stride() == s);
			REQUIRE(gen0.format().array_at(0).offset() == 0);
			REQUIRE(gen0.format().array_at(0).elem_size() == sizeof(int));
			REQUIRE(gen0.format().array_at(0).elem_alignment() == alignof(int));
			REQUIRE(gen0.format().array_at(0).elem_padding() == pad);

			ndarray_view<3, int> re = from_generic<3, int>(gen0, make_ndsize(3, 4, 4));
			REQUIRE(same(re, arr));

			REQUIRE_THROWS(( from_generic<3, int>(gen0, make_ndsize(3, 4, 5)) ));
		}

		SECTION("generic dimension 1") {
			ndarray_view_generic<1> gen1 = to_generic<1>(arr);
			REQUIRE(gen1.start() == reinterpret_cast<byte*>(arr.start()));

			REQUIRE(gen1.shape() == make_ndsize(3));
			REQUIRE(gen1.strides() == head<1>(str));

			REQUIRE(gen1.format().is_defined());
			REQUIRE(gen1.format().frame_size() == shp[1]*str[1]);
			REQUIRE(gen1.format().frame_alignment_requirement() == alignof(int));
			REQUIRE(gen1.format().arrays_count() == 1);
			REQUIRE(gen1.format().array_at(0).is_defined());
			REQUIRE(gen1.format().array_at(0).frame_size() == shp[1]*str[1]);
			REQUIRE(gen1.format().array_at(0).frame_alignment_requirement() == alignof(int));
			REQUIRE(gen1.format().array_at(0).elem_count() == tail<2>(shp).product());
			REQUIRE(gen1.format().array_at(0).elem_stride() == s);
			REQUIRE(gen1.format().array_at(0).offset() == 0);
			REQUIRE(gen1.format().array_at(0).elem_size() == sizeof(int));
			REQUIRE(gen1.format().array_at(0).elem_alignment() == alignof(int));
			REQUIRE(gen1.format().array_at(0).elem_padding() == pad);

			ndarray_view<3, int> re = from_generic<3, int>(gen1, make_ndsize(4, 4));
			REQUIRE(same(re, arr));
			
			REQUIRE_THROWS(( from_generic<3, int>(gen1, make_ndsize(4, 5)) ));

			ndarray_view_generic<0> gen0 = gen1[0];
			REQUIRE(gen0.shape() == make_ndsize());
			REQUIRE(gen0.strides() == make_ndptrdiff());
		}
		
		SECTION("generic dimension 2") {
			ndarray_view_generic<2> gen2 = to_generic<2>(arr);
			REQUIRE(gen2.start() == reinterpret_cast<byte*>(arr.start()));
			
			REQUIRE(gen2.shape() == head<2>(shp));
			REQUIRE(gen2.strides() == head<2>(str));

			REQUIRE(gen2.format().is_defined());
			REQUIRE(gen2.format().frame_size() == shp[2]*str[2]);
			REQUIRE(gen2.format().frame_alignment_requirement() == alignof(int));
			REQUIRE(gen2.format().arrays_count() == 1);
			REQUIRE(gen2.format().array_at(0).is_defined());
			REQUIRE(gen2.format().array_at(0).frame_size() == shp[2]*str[2]);
			REQUIRE(gen2.format().array_at(0).frame_alignment_requirement() == alignof(int));
			REQUIRE(gen2.format().array_at(0).elem_count() == shp[2]);
			REQUIRE(gen2.format().array_at(0).elem_stride() == s);
			REQUIRE(gen2.format().array_at(0).offset() == 0);
			REQUIRE(gen2.format().array_at(0).elem_size() == sizeof(int));
			REQUIRE(gen2.format().array_at(0).elem_alignment() == alignof(int));
			REQUIRE(gen2.format().array_at(0).elem_padding() == pad);

			ndarray_view<3, int> re = from_generic<3, int>(gen2, make_ndsize(4));
			REQUIRE(same(re, arr));

			REQUIRE_THROWS(( from_generic<3, int>(gen2, make_ndsize(5)) ));
			
			ndarray_view_generic<1> gen1 = gen2[0];
			REQUIRE(gen1.shape() == make_ndsize(4));
			REQUIRE(gen1.strides() == make_ndptrdiff(str[1]));
		}
	}
	

	SECTION("partial default stride, with padding") {
		shp[0] = 2;
		str[0] *= 2;
		ndarray_view<3, int> arr(raw.data(), shp, str);
		std::size_t s = arr.strides().back();
		
		SECTION("generic dimension 0") {
			REQUIRE_THROWS(to_generic<0>(arr));
		}

		SECTION("generic dimension 1") {
			ndarray_view_generic<1> gen1 = to_generic<1>(arr);
			REQUIRE(gen1.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen1.shape() == head<1>(shp));
			REQUIRE(gen1.strides() == head<1>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen1, make_ndsize(4, 4));
			REQUIRE(same(re, arr));
		}
		
		SECTION("generic dimension 2") {
			ndarray_view_generic<2> gen2 = to_generic<2>(arr);
			REQUIRE(gen2.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen2.shape() == head<2>(shp));
			REQUIRE(gen2.strides() == head<2>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen2, make_ndsize(4));
			REQUIRE(same(re, arr));
			
			ndarray_view_generic<1> gen1 = gen2[0];
			REQUIRE(gen1.shape() == make_ndsize(4));
			REQUIRE(gen1.strides() == make_ndptrdiff(str[1]));
		}
	}
}
