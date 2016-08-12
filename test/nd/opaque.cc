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

TEST_CASE("ndarray_opaque_view", "[nd][ndarray_opaque_view]") {
	constexpr std::size_t l = sizeof(int);

	SECTION("basics") {
		// default strides
		{
		ndarray_opaque_frame_format frm( make_ndarray_format<int>(100) );
		ndarray_opaque_frame_format frm_str( make_ndarray_format<int>(100, 2*l) );
	
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm) ==
			make_ndptrdiff(2*100*l, 100*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm, 3*l) ==
			make_ndptrdiff(2*103*l, 103*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm_str) ==
			make_ndptrdiff(2*100*2*l, 100*2*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm_str, 3*l) ==
			make_ndptrdiff( 2*(100*2*l + 3*l ), 100*2*l + 3*l ) ));
		}
		
		// default strides view
		// frame length=10, stride=2*l, frame_padding=3*l
		std::unique_ptr<int[]> buffer(new int[3 * 2 * (10*2 + 3)]);
		ndarray_opaque_frame_format frm(make_ndarray_format<int>(10, 2*l));
		auto shp = make_ndsize(3, 2);
		auto str = ndarray_view_opaque<2>::default_strides(3*l, frm);
		auto start = static_cast<ndarray_view_opaque<2>::frame_ptr>(buffer.get());
		ndarray_view_opaque<2> vw(start, shp, str, frm);
		REQUIRE(vw.start() == start);
		REQUIRE(vw.shape() == shp);
		REQUIRE(vw.strides() == str);
		
		// TODO reset, same, null, etc basics
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

