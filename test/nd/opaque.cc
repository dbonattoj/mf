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
#include <mf/os/memory.h>
#include "../support/ndarray_opaque.h"

using namespace mf;
using namespace mf::test;

TEST_CASE("ndarray_view_opaque", "[nd][ndarray_view_opaque]") {
	constexpr std::size_t l = sizeof(int);

	SECTION("default strides") {
		ndarray_opaque_frame_format frm( make_ndarray_format<int>(100) );
		ndarray_opaque_frame_format frm_str( make_ndarray_format<int>(100, 2*l) );
		REQUIRE(frm.frame_size() == 100*sizeof(int));
	
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm) ==
			make_ndptrdiff(2*100*l, 100*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm, 3*l) ==
			make_ndptrdiff(2*103*l, 103*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm_str) ==
			make_ndptrdiff(2*100*2*l, 100*2*l) ));
		REQUIRE(( ndarray_view_opaque<2>::default_strides(make_ndsize(3, 2), frm_str, 3*l) ==
			make_ndptrdiff( 2*(100*2*l + 3*l ), 100*2*l + 3*l ) ));
	}
	
	SECTION("contiguous") {
		// creating two views to different data
		auto shp = make_ndsize(2, 3);
		std::vector<int> raw1(2 * 3 * 10);
		std::vector<int> raw2 = raw1;
		for(std::ptrdiff_t i = 0; i != raw1.size(); ++i) {
			raw1[i] = i;
			raw2[i] = 2*i + 1;
		}
		ndarray_opaque_frame_format frm(make_ndarray_format<int>(10));
		auto str = ndarray_view_opaque<2>::default_strides(shp, frm);
		ndarray_view_opaque<2> vw1(static_cast<void*>(&raw1[0]), shp, str, frm);
		ndarray_view_opaque<2> vw2(static_cast<void*>(&raw2[0]), shp, str, frm);
		
		// assign and compare 0-d section
		REQUIRE(vw1 == vw1);
		REQUIRE(vw1 != vw2);

		vw1[1][1] = vw2[1][1];
		REQUIRE(vw1[1][0] != vw2[1][0]);
		REQUIRE(vw1[1][1] == vw2[1][1]);
		REQUIRE(vw1[1][2] != vw2[1][2]);
		
		REQUIRE_FALSE(raw1[(1*3 + 1)*10 - 1] == raw2[(1*3 + 1)*10 - 1]);
		for(std::ptrdiff_t i = 0; i < 10; ++i)
			REQUIRE(raw1[(1*3 + 1)*10 + i] == raw2[(1*3 + 1)*10 + i]);
		REQUIRE_FALSE(raw1[(1*3 + 1)*10 + 10] == raw2[(1*3 + 1)*10 + 10]);
		
		// assign and compare 1-d section
		vw1[0] = vw2[0];
		REQUIRE(vw1[0] == vw2[0]);
		REQUIRE(vw1[1] != vw2[1]);
		
		// assign and compare full
		vw1 = vw2;
		REQUIRE(vw1 == vw2);
	}
	
	
	SECTION("non-contiguous") {
		// 4 kinds of padding:
		// (1) between array elements in frame part
		// (2) between frame parts (to satisfy alignment of frame part array elements)
		// (3) at the end of frame (to satisfy alignment of frame)
		// (4) additional stride between frames
		
		// creating two views to different data
		auto shp = make_ndsize(2, 3);
		auto frm = opaque_frame_format();
		raw_allocator alloc;
		std::size_t f = frm.frame_size(); // frame size
		std::size_t p = 32; // additional stride padding between frames (4)
		std::size_t fp = frm.frame_size() + p; // frame size with stride padding
		std::size_t n = 2 * 3 * fp; // full length of array memory
		void* raw1_v = alloc.raw_allocate(n, frm.frame_alignment_requirement());
		void* raw2_v = alloc.raw_allocate(n, frm.frame_alignment_requirement());
		
		byte* raw1 = static_cast<byte*>(raw1_v);
		byte* raw2 = static_cast<byte*>(raw2_v);
		for(std::ptrdiff_t i = 0; i < n; ++i) {
			raw1[i] = (i % 100);
			raw2[i] = 100 + ((2*i + 1) % 100);
		}
		auto str = make_ndptrdiff(3*fp ,fp);
		
		ndarray_view_opaque<2> vw1(raw1_v, shp, str, frm);
		ndarray_view_opaque<2> vw2(raw2_v, shp, str, frm);
		
		// assign and compare 0-d section
		REQUIRE(vw1 == vw1);
		REQUIRE(vw1 != vw2);

		vw1[1][1] = vw2[1][1];
		REQUIRE(vw1[1][0] != vw2[1][0]);
		REQUIRE(vw1[1][1] == vw2[1][1]);
		REQUIRE(vw1[1][2] != vw2[1][2]);
		
		// assign and compare 1-d section
		vw1[0] = vw2[0];
		REQUIRE(vw1[0] == vw2[0]);
		REQUIRE(vw1[1] != vw2[1]);
		
		// assign and compare full
		vw1 = vw2;
		REQUIRE(vw1 == vw2);
		
		// write in padding
		REQUIRE(frm.part_at(1).format.elem_padding() > 1);
		raw1[frm.part_at(1).offset + frm.part_at(1).format.elem_size()] = 244; // (1)
		raw1[frm.part_at(1).offset - 1] = 244; // (2)
		raw1[frm.part_at(2).offset + frm.part_at(2).format.frame_size()] = 244; // (3)
		raw1[frm.frame_size() + 1] = 244; // (4);
		REQUIRE(vw1 == vw2); // must still be equal (compare does not look in padidng area)
		vw1 = vw2;
		// must not have overwritte padding areas:
		REQUIRE(244 == raw1[frm.part_at(1).offset + frm.part_at(1).format.elem_size()]);
		REQUIRE(244 == raw1[frm.part_at(1).offset - 1]);
		REQUIRE(244 == raw1[frm.part_at(2).offset + frm.part_at(2).format.frame_size()]);
		REQUIRE(244 == raw1[frm.frame_size() + 1]);
		
		// deallocate
		alloc.raw_deallocate(raw1_v, n);
		alloc.raw_deallocate(raw2_v, n);
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

