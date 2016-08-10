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
#include <mf/ndarray/opaque/ndarray_view_opaque.h>
#include <mf/ndarray/opaque/ndarray_timed_view_opaque.h>
#include <mf/ndarray/opaque/ndarray_opaque.h>

using namespace mf;


TEST_CASE("ndarray_opaque_frame_format", "[nd][ndarray_opaque_frame_format]") {
	SECTION("undefined") {
		ndarray_opaque_frame_format frm;
		REQUIRE_FALSE(frm.is_defined());
	}

	SECTION("single-part") {
		ndarray_format afrm = make_ndarray_format<int>(100);
		ndarray_opaque_frame_format frm(afrm);
		
		REQUIRE(frm.is_defined());
		REQUIRE(frm.is_single_part());
		REQUIRE_FALSE(frm.is_multi_part());
		REQUIRE_FALSE(frm.is_raw());
		REQUIRE(frm.parts_count() == 1);
		
		REQUIRE(frm.frame_size() == 100 * sizeof(int));
		REQUIRE(frm.frame_alignment_requirement() == alignof(int));
		REQUIRE(frm.part_at(0).offset == 0);
		REQUIRE(frm.part_at(0).format == afrm);
		REQUIRE(frm.array_format() == afrm);
		
		ndarray_opaque_frame_format frm2;
		frm2.add_part(afrm);
		REQUIRE(frm == frm2);
	}

	SECTION("multi-part") {
		ndarray_opaque_frame_format frm;
		ndarray_format afrm1 = make_ndarray_format<int>(100);
		ndarray_format afrm2 = make_ndarray_format<double>(50, 2*sizeof(double));
		
		frm.add_part(afrm1);
		frm.add_part(afrm2);
		
		REQUIRE(frm.is_defined());
		REQUIRE_FALSE(frm.is_single_part());
		REQUIRE_FALSE(frm.is_raw());
		REQUIRE(frm.is_multi_part());
		REQUIRE(frm.parts_count() == 2);
		REQUIRE_THROWS(frm.array_format());
		REQUIRE(frm.part_at(0).format == afrm1);
		REQUIRE(frm.part_at(0).offset == 0);
		REQUIRE(frm.part_at(1).format == afrm2);
		REQUIRE(frm.part_at(1).offset > 0);
		REQUIRE(frm.frame_size() >= afrm1.frame_size() + afrm2.frame_size());
		REQUIRE(frm.frame_alignment_requirement() >= std::max(afrm1.elem_alignment(), afrm2.elem_alignment()));
	}
	
	SECTION("raw") {
		ndarray_opaque_frame_format frm(1024, 2);
		REQUIRE(frm.is_defined());
		REQUIRE_FALSE(frm.is_single_part());
		REQUIRE_FALSE(frm.is_multi_part());
		REQUIRE(frm.is_raw());
		REQUIRE(frm.parts_count() == 0);
		REQUIRE(frm.frame_size() == 1024);
		REQUIRE(frm.frame_alignment_requirement() == 2);
		REQUIRE_THROWS(frm.part_at(0));
		REQUIRE_THROWS(frm.array_format());
	}
}



TEST_CASE("ndarray_view <--> ndarray_view_opaque", "[nd][ndarray_view_opaque]") {
	constexpr std::size_t pad = sizeof(int);
	constexpr std::size_t len = 3 * 4 * 4;
	std::vector<int> raw(2 * len);
	for(int i = 0; i < 2*len; i += 2) raw[i] = i;
	auto shp = make_ndsize(3, 4, 4);
	auto str = ndarray_view<3, int>::default_strides(shp, pad);
	
	SECTION("default stride, with padding") {
		ndarray_view<3, int> vw(raw.data(), shp, str);
		std::size_t s = vw.strides().back();

		SECTION("opaque dimension 0") {
			ndarray_view_opaque<0> op0 = to_opaque<0>(vw);
			REQUIRE(op0.start() == reinterpret_cast<byte*>(vw.start()));
			REQUIRE(op0.shape() == make_ndsize());
			REQUIRE(op0.strides() == make_ndptrdiff());
			
			REQUIRE(op0.format().is_defined());
			REQUIRE(op0.format().is_single_part());
			REQUIRE(op0.format().array_format() == format(vw));

			ndarray_view<3, int> re = from_opaque<3, int>(op0, make_ndsize(3, 4, 4));
			REQUIRE(same(re, vw));

			REQUIRE_THROWS(( from_opaque<3, int>(op0, make_ndsize(3, 4, 5)) ));
		}

		SECTION("opaque dimension 1") {
			ndarray_view_opaque<1> op1 = to_opaque<1>(vw);
			REQUIRE(op1.start() == reinterpret_cast<byte*>(vw.start()));

			REQUIRE(op1.shape() == make_ndsize(3));
			REQUIRE(op1.strides() == head<1>(str));

			REQUIRE(op1.format().is_defined());
			REQUIRE(op1.format().is_single_part());
			REQUIRE(op1.format().array_format() == tail_format<2>(vw));

			ndarray_view<3, int> re = from_opaque<3, int>(op1, make_ndsize(4, 4));
			REQUIRE(same(re, vw));
			
			REQUIRE_THROWS(( from_opaque<3, int>(op1, make_ndsize(4, 5)) ));

			ndarray_view_opaque<0> op0 = op1[0];
			REQUIRE(op0.shape() == make_ndsize());
			REQUIRE(op0.strides() == make_ndptrdiff());
		}

		SECTION("opaque dimension 2") {
			ndarray_view_opaque<2> op2 = to_opaque<2>(vw);
			REQUIRE(op2.start() == reinterpret_cast<byte*>(vw.start()));
			
			REQUIRE(op2.shape() == head<2>(shp));
			REQUIRE(op2.strides() == head<2>(str));

			REQUIRE(op2.format().is_defined());
			REQUIRE(op2.format().is_single_part());
			REQUIRE(op2.format().array_format() == tail_format<1>(vw));

			ndarray_view<3, int> re = from_opaque<3, int>(op2, make_ndsize(4));
			REQUIRE(same(re, vw));

			REQUIRE_THROWS(( from_opaque<3, int>(op2, make_ndsize(5)) ));
			
			ndarray_view_opaque<1> op1 = op2[0];
			REQUIRE(op1.shape() == make_ndsize(4));
			REQUIRE(op1.strides() == make_ndptrdiff(str[1]));
		}
	}
	

	SECTION("partial default stride, with padding") {
		shp[0] = 2;
		str[0] *= 2;
		ndarray_view<3, int> vw(raw.data(), shp, str);
		std::size_t s = vw.strides().back();
		
		SECTION("opaque dimension 0") {
			REQUIRE_THROWS(to_opaque<0>(vw));
		}

		SECTION("opaque dimension 1") {
			ndarray_view_opaque<1> op1 = to_opaque<1>(vw);
			REQUIRE(op1.start() == reinterpret_cast<byte*>(vw.start()));
			REQUIRE(op1.shape() == head<1>(shp));
			REQUIRE(op1.strides() == head<1>(str));

			REQUIRE(op1.format().is_defined());
			REQUIRE(op1.format().is_single_part());
			REQUIRE(op1.format().array_format() == tail_format<2>(vw));

			ndarray_view<3, int> re = from_opaque<3, int>(op1, make_ndsize(4, 4));
			REQUIRE(same(re, vw));
		}
		
		SECTION("opaque dimension 2") {
			ndarray_view_opaque<2> op2 = to_opaque<2>(vw);
			REQUIRE(op2.start() == reinterpret_cast<byte*>(vw.start()));
			REQUIRE(op2.shape() == head<2>(shp));
			REQUIRE(op2.strides() == head<2>(str));

			REQUIRE(op2.format().is_defined());
			REQUIRE(op2.format().is_single_part());
			REQUIRE(op2.format().array_format() == tail_format<1>(vw));

			ndarray_view<3, int> re = from_opaque<3, int>(op2, make_ndsize(4));
			REQUIRE(same(re, vw));
			
			ndarray_view_opaque<1> op1 = op2[0];
			REQUIRE(op1.shape() == make_ndsize(4));
			REQUIRE(op1.strides() == make_ndptrdiff(str[1]));
		}
	}
}

