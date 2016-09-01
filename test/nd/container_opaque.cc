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
#include <mf/nd/opaque/ndarray_view_opaque.h>
#include <mf/nd/opaque/ndarray_opaque.h>
#include "../support/ndarray_opaque.h"

using namespace mf;
using namespace mf::test;

static void verify_ndarray_memory_(ndarray_opaque<2>& arr) {
	const auto& shp = arr.shape();
	int i = 0;
	
	for(const auto& coord : make_ndspan(shp))
		arr.at(coord) = make_opaque_frame(i++);
	
	i = 0;
	for(const auto& coord : make_ndspan(shp))
		REQUIRE(arr.at(coord) == make_opaque_frame(i++));
}


TEST_CASE("ndarray_opaque", "[nd][ndarray_opaque]") {
	auto frm = opaque_frame_format();

	constexpr std::ptrdiff_t pad = 64;
	ndsize<2> shape{3, 4};
	ndarray_opaque<2> og_arr(shape, frm, 0);
	ndarray_view_opaque<2> arr_vw = og_arr.view();

	ndarray_view_opaque<2> arr_vw_sec = arr_vw()(0, 2);
	REQUIRE_FALSE(arr_vw_sec.has_default_strides());


	SECTION("construction") {
		// construction with shape
		ndarray_opaque<2> arr(shape, frm);
		REQUIRE(arr.shape() == shape);
		REQUIRE((arr.strides() == ndarray_view_opaque<2>::default_strides(shape, frm)));
		REQUIRE(arr.allocated_byte_size() >= arr_vw.size()*frm.frame_size());
		verify_ndarray_memory_(arr);

		// construction with shape, padding
		ndarray_opaque<2> arr_pad(shape, frm, pad);
		REQUIRE(arr_pad.shape() == shape);
		REQUIRE((arr_pad.strides() == ndarray_view_opaque<2>::default_strides(shape, frm, pad)));
		verify_ndarray_memory_(arr_pad);

		// construction from view (ndarray gets default strides)
		ndarray_opaque<2> arr2(arr_vw_sec);
		REQUIRE(arr2.view().compare(arr_vw_sec));
		REQUIRE(arr2.shape() == arr_vw_sec.shape());
		REQUIRE((arr2.strides() == ndarray_view_opaque<2>::default_strides(arr_vw_sec.shape(), frm)));
		verify_ndarray_memory_(arr2);
		arr2[1][1] = make_opaque_frame(456);
		REQUIRE(arr2[1][1] == make_opaque_frame(456));
		REQUIRE_FALSE(arr_vw[1][1] == make_opaque_frame(456));

		// construction from view (ndarray gets padded default strides)
		ndarray_opaque<2> arr3(arr_vw_sec, pad);
		REQUIRE(arr3.view().compare(arr_vw_sec));
		REQUIRE(arr3.shape() == arr_vw_sec.shape());
		REQUIRE((arr3.strides() == ndarray_view_opaque<2>::default_strides(arr_vw_sec.shape(), frm, pad)));
		verify_ndarray_memory_(arr3);
		arr3[1][1] = make_opaque_frame(456);
		REQUIRE(arr3[1][1] == make_opaque_frame(456));
		REQUIRE_FALSE(arr_vw[1][1] == make_opaque_frame(456));

		// construction from null ndarray_view
		//REQUIRE_THROWS(new ndarray_opaque<2>(ndarray_view_opaque<2>::null())); Catch bug?
	
		// copy-construction from another ndarray (strides get copied)
		ndarray_opaque<2> arr4 = arr3;
		REQUIRE(arr4.view().compare(arr3));
		REQUIRE(arr4.shape() == arr3.shape());
		REQUIRE(arr4.strides() == arr3.strides());
		REQUIRE(arr4[1][1] == make_opaque_frame(456));
		verify_ndarray_memory_(arr4);
		arr4[1][1] = make_opaque_frame(789);
		REQUIRE(arr4[1][1] == make_opaque_frame(789));
		REQUIRE_FALSE(arr3[1][1] == make_opaque_frame(789));
	
		// move construction from another ndarray (strides get copied)
		ndarray_opaque<2> arr5_cmp = arr4;
		ndarray_opaque<2> arr5 = std::move(arr4);
		REQUIRE(arr5.view().compare(arr5_cmp));
		REQUIRE(arr5.shape() == arr5_cmp.shape());
		REQUIRE(arr5.strides() == arr5_cmp.strides());
		verify_ndarray_memory_(arr5);
	}


	SECTION("assignment") {
		ndsize<2> previous_shape{5, 1};
		ndsize<2> shp = arr_vw_sec.shape();
		
		// assignment from view (ndarray gets default strides)
		ndarray_opaque<2> arr_(previous_shape, frm), arr_2(previous_shape, frm);
		arr_ = arr_vw_sec;
		REQUIRE(arr_.view().compare(arr_vw_sec));
		REQUIRE(arr_.shape() == arr_vw_sec.shape());
		REQUIRE((arr_.strides() == ndarray_view_opaque<2>::default_strides(shp, frm)));
		arr_[1][1] = make_opaque_frame(123);
		REQUIRE_FALSE(arr_vw[1][1] == make_opaque_frame(123));
		arr_2.assign(arr_vw_sec);
		REQUIRE(arr_2.view().compare(arr_vw_sec));
		REQUIRE(arr_2.shape() == arr_vw_sec.shape());
		REQUIRE((arr_2.strides() == ndarray_view_opaque<2>::default_strides(shp, frm)));
		arr_2[1][1] = make_opaque_frame(123);
		REQUIRE_FALSE(arr_vw[1][1] == make_opaque_frame(123));
				
		// assignment from view (ndarray gets padded default strides)
		ndarray_opaque<2> arr_3(previous_shape, frm);
		arr_3.assign(arr_vw_sec, pad);
		REQUIRE(arr_3.view().compare(arr_vw_sec));
		REQUIRE(arr_3.shape() == arr_vw_sec.shape());
		REQUIRE((arr_3.strides() == ndarray_view_opaque<2>::default_strides(shp, frm, pad)));
		arr_3[1][1] = make_opaque_frame(456);
		REQUIRE_FALSE(arr_vw_sec[1][1] == make_opaque_frame(456));

		// assignment from null ndarray_view
		ndarray_opaque<2> arr_4(previous_shape, frm), arr_4_(previous_shape, frm);
		REQUIRE_THROWS(arr_4 = ndarray_view_opaque<2>::null());
		REQUIRE_THROWS(arr_4_.assign(ndarray_view_opaque<2>::null()));

		// copy-assignment from another ndarray (strides get copied)
		ndarray_opaque<2> arr_5(previous_shape, frm);
		arr_5 = arr_3;
		REQUIRE(arr_5.view().compare(arr_3));
		REQUIRE(arr_5.shape() == arr_3.shape());
		REQUIRE(arr_5.strides() == arr_3.strides());
		arr_5[1][1] = make_opaque_frame(789);
		REQUIRE_FALSE(arr_3[1][1] == make_opaque_frame(789));
		
		// move-assignment from another ndarray (strides get copied)
		ndarray_opaque<2> arr_7(previous_shape, frm);
		ndarray_opaque<2> arr_3_cmp = arr_3;
		arr_7 = std::move(arr_3);
		REQUIRE(arr_7.view().compare(arr_3_cmp));
		REQUIRE(arr_7.shape() == arr_3_cmp.shape());
		REQUIRE(arr_7.strides() == arr_3_cmp.strides());
		verify_ndarray_memory_(arr_7);
	}
}
