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

using namespace mf;

TEST_CASE("ndarray view, generic", "[ndarray][generic]") {
	constexpr std::size_t l = sizeof(int);
	constexpr std::size_t len = 3 * 4 * 4;
	std::vector<int> raw(2 * len);
	for(int i = 0; i < 2*len; i += 2) raw[i] = i;
	auto shp = make_ndsize(3, 4, 4);
	auto str = ndarray_view<3, int>::default_strides(shp, l);
	
	SECTION("default stride, with padding") {
		ndarray_view<3, int> arr(raw.data(), shp, str);
		std::size_t s = arr.strides().back();
				
		SECTION("generic dimension 0") {
			ndarray_view_generic<0> gen0 = to_generic<0>(arr);
			REQUIRE(gen0.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen0.generic_shape() == make_ndsize());
			REQUIRE(gen0.generic_strides() == make_ndptrdiff());
			
			const frame_format& frm = gen0.format();
			REQUIRE(frm.is_defined());
			REQUIRE(frm.frame_size() == shp.front() * str.front());
			REQUIRE(frm.frame_alignment_requirement() == alignof(int));
			REQUIRE(frm.arrays_count() == 1);
			const frame_array_format& afrm = frm.array_at(0);
			REQUIRE(afrm.is_defined());
			REQUIRE(afrm.frame_size() == shp.front() * str.front());
			REQUIRE(afrm.frame_alignment_requirement() == alignof(int));
			REQUIRE(afrm.elem_count() == shp.product());
			REQUIRE(afrm.elem_stride() == s);
			REQUIRE(afrm.offset() == 0);
			REQUIRE(afrm.elem_size() == sizeof(int));
			REQUIRE(afrm.elem_alignment() == alignof(int));
			REQUIRE(afrm.elem_padding() == l);

			ndarray_view<3, int> re = from_generic<3, int>(gen0, make_ndsize(3, 4, 4));
			REQUIRE(same(re, arr));

			REQUIRE_THROWS(( from_generic<3, int>(gen0, make_ndsize(3, 4, 5)) ));
		}

		SECTION("generic dimension 1") {
			ndarray_view_generic<1> gen1 = to_generic<1>(arr);
			REQUIRE(gen1.start() == reinterpret_cast<byte*>(arr.start()));

			REQUIRE(gen1.generic_shape() == make_ndsize(3));
			REQUIRE(gen1.generic_strides() == head<1>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen1, make_ndsize(4, 4));
			REQUIRE(same(re, arr));
			
			REQUIRE_THROWS(( from_generic<3, int>(gen1, make_ndsize(4, 5)) ));

			ndarray_view_generic<0> gen0 = gen1[0];
			REQUIRE(gen0.generic_shape() == make_ndsize());
			REQUIRE(gen0.generic_strides() == make_ndptrdiff());
		}
		
		SECTION("generic dimension 2") {
			ndarray_view_generic<2> gen2 = to_generic<2>(arr);
			REQUIRE(gen2.start() == reinterpret_cast<byte*>(arr.start()));
			
			REQUIRE(gen2.generic_shape() == head<2>(shp));
			REQUIRE(gen2.generic_strides() == head<2>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen2, make_ndsize(4));
			REQUIRE(same(re, arr));

			REQUIRE_THROWS(( from_generic<3, int>(gen2, make_ndsize(5)) ));
			
			ndarray_view_generic<1> gen1 = gen2[0];
			REQUIRE(gen1.generic_shape() == make_ndsize(4));
			REQUIRE(gen1.generic_strides() == make_ndptrdiff(str[1]));
		}
	}
	

	SECTION("partial default stride, with padding") {
		shp[0] = 2;
		str[0] *= 2;
		ndarray_view<3, int> arr(raw.data(), shp, str);
		std::size_t s = arr.strides().back();

		SECTION("generic dimension 1") {
			ndarray_view_generic<1> gen1 = to_generic<1>(arr);
			REQUIRE(gen1.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen1.generic_shape() == head<1>(shp));
			REQUIRE(gen1.generic_strides() == head<1>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen1, make_ndsize(4, 4));
			REQUIRE(same(re, arr));
		}
		
		SECTION("generic dimension 2") {
			ndarray_view_generic<2> gen2 = to_generic<2>(arr);
			REQUIRE(gen2.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen2.generic_shape() == head<2>(shp));
			REQUIRE(gen2.generic_strides() == head<2>(str));

			ndarray_view<3, int> re = from_generic<3, int>(gen2, make_ndsize(4));
			REQUIRE(same(re, arr));
			
			ndarray_view_generic<1> gen1 = gen2[0];
			REQUIRE(gen1.generic_shape() == make_ndsize(4));
			REQUIRE(gen1.generic_strides() == make_ndptrdiff(str[1]));
		}
	}


	SECTION("partial default stride, with padding, timed") {
		shp[0] = 2;
		str[0] *= 2;
		ndarray_view<3, int> arr_(raw.data(), shp, str);
		ndarray_timed_view<3, int> arr(arr_, 100);
		std::size_t s = arr.strides().back();

		SECTION("generic dimension 1") {
			ndarray_timed_view_generic<1> gen1 = to_generic<1>(arr);
			REQUIRE(gen1.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen1.generic_shape() == head<1>(shp));
			REQUIRE(gen1.generic_strides() == head<1>(str));

			ndarray_timed_view<3, int> re = from_generic<3, int>(gen1, make_ndsize(4, 4));
			REQUIRE(same(re, arr));
		}
		
		SECTION("generic dimension 2") {
			ndarray_timed_view_generic<2> gen2 = to_generic<2>(arr);
			REQUIRE(gen2.start() == reinterpret_cast<byte*>(arr.start()));
			REQUIRE(gen2.generic_shape() == head<2>(shp));
			REQUIRE(gen2.generic_strides() == head<2>(str));

			ndarray_timed_view<3, int> re = from_generic<3, int>(gen2, make_ndsize(4));
			REQUIRE(same(re, arr));
			
			ndarray_view_generic<1> gen1 = gen2[0];
			REQUIRE(gen1.generic_shape() == make_ndsize(4));
			REQUIRE(gen1.generic_strides() == make_ndptrdiff(str[1]));
		}
	}
}
