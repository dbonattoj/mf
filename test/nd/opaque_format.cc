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
#include <mf/ndarray/opaque/ndarray_opaque_frame_format.h>

using namespace mf;

using unaligned_int32 = std::array<byte, 4>;
using unaligned_int64 = std::array<byte, 8>;

unaligned_int32 u32(std::int32_t i) { return *reinterpret_cast<unaligned_int32*>(&i); };
unaligned_int64 u64(std::int64_t i) { return *reinterpret_cast<unaligned_int64*>(&i); };


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
		REQUIRE(frm.is_contiguous());
		
		ndarray_opaque_frame_format frm2;
		frm2.add_part(afrm);
		REQUIRE(frm == frm2);
		
		std::vector<int> raw1(100), raw2(100);
		for(std::ptrdiff_t i = 0; i < 100; ++i) {
			raw1[i] = i;
			raw2[i] = 2*i + 1;
		}

		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw1.data()),
			frm
		));
		REQUIRE_FALSE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw2.data()),
			frm
		));
		ndarray_opaque_frame_copy(
			static_cast<void*>(&raw2[0]),
			static_cast<const void*>(raw1.data()),
			frm
		);
		REQUIRE(raw1 == raw2);
		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw2.data()),
			frm
		));
	}


	SECTION("multi-part") {
		ndarray_opaque_frame_format frm;
		ndarray_format afrm1 = make_ndarray_format<std::int32_t>(49);
		ndarray_format afrm2 = make_ndarray_format<std::int64_t>(20, 2*8);
		ndarray_format afrm3 = make_ndarray_format<std::int8_t>(3);
	
		REQUIRE(sizeof(std::int32_t) == alignof(std::int32_t));
		REQUIRE(sizeof(std::int64_t) == alignof(std::int64_t));
		
		frm.add_part(afrm1);
		frm.add_part(afrm2);
		frm.add_part(afrm3);
		
		REQUIRE(frm.is_defined());
		REQUIRE_FALSE(frm.is_single_part());
		REQUIRE_FALSE(frm.is_raw());
		REQUIRE(frm.is_multi_part());
		REQUIRE(frm.parts_count() == 3);
		REQUIRE_THROWS(frm.array_format());
		
		REQUIRE(frm.part_at(0).format == afrm1);
		REQUIRE(frm.part_at(0).offset == 0);
		REQUIRE(frm.part_at(1).format == afrm2);
		REQUIRE(frm.part_at(1).offset == 49*4 + 4);
		REQUIRE(frm.part_at(2).format == afrm3);
		REQUIRE(frm.part_at(2).offset == (49*4 + 4) + 20*2*8);
		REQUIRE(frm.frame_size() == (49*4 + 4) + (20*2*8) + (3*1));
		REQUIRE(frm.frame_size_with_padding() == (49*4 + 4) + (20*2*8) + 8);
		REQUIRE(is_multiple_of(frm.frame_size_with_padding(), frm.frame_alignment_requirement()));
		REQUIRE(frm.frame_alignment_requirement() == 8);

		struct padded_int64_t {
			unaligned_int64 data;
			std::array<byte, 8> pad;
		
			padded_int64_t() = default;
			padded_int64_t(std::int64_t dat, byte pd) : data(u64(dat)) { pad.fill(pd); }
			
			bool operator==(const padded_int64_t& other) const { return data == other.data; }
		};

		struct frame_t {			
			std::array<unaligned_int32, 49> part1;
					std::array<byte, 4> pad1;
			std::array<padded_int64_t, 20> part2;
			std::array<byte, 3> part3;
					std::array<byte, 8> pad2;
					
			bool operator==(const frame_t& other) const {
				return
					(part1 == other.part1) &&
					(part2 == other.part2) &&
					(part3 == other.part3);
			}
		};
		
		REQUIRE(offsetof(frame_t, part1) == 0);
		REQUIRE(offsetof(frame_t, part2) == frm.part_at(1).offset);
		REQUIRE(offsetof(frame_t, part3) == frm.part_at(2).offset);
		
		frame_t frame1, frame2;
		frame1.part1.fill(u32(0x12345678));
		frame1.pad1.fill(0);
		frame1.part2.fill(padded_int64_t(0xABCDFFFFABCDFFFF, 0));
		frame1.part3.fill(0xDD);
		frame1.pad2.fill(0);		
		
		frame2.part1.fill(u32(0x87654321));
		frame2.pad1.fill(7);
		frame2.part2.fill(padded_int64_t(0xEFABEFABEFABEFAB, 7));
		frame2.part3.fill(0x11);
		frame2.pad2.fill(7);
		
		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(&frame1),
			static_cast<const void*>(&frame1),
			frm
		));
		REQUIRE_FALSE(ndarray_opaque_frame_compare(
			static_cast<const void*>(&frame1),
			static_cast<const void*>(&frame2),
			frm
		));
		ndarray_opaque_frame_copy(
			static_cast<void*>(&frame2),
			static_cast<const void*>(&frame1),
			frm
		);

		REQUIRE(frame1 == frame2);	
		REQUIRE(frame2.pad1[0] == 7);
		REQUIRE(frame2.part2[0].pad[0] == 7);
		REQUIRE(frame2.pad2[0] == 7);
		
		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(&frame1),
			static_cast<const void*>(&frame2),
			frm
		));
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
		

		std::vector<byte> raw1(1024), raw2(1024);
		for(std::ptrdiff_t i = 0; i < 1024; ++i) {
			raw1[i] = i % 255;
			raw2[i] = (2*i + 1) % 255;;
		}

		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw1.data()),
			frm
		));
		REQUIRE_FALSE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw2.data()),
			frm
		));
		ndarray_opaque_frame_copy(
			static_cast<void*>(&raw2[0]),
			static_cast<const void*>(raw1.data()),
			frm
		);
		REQUIRE(raw1 == raw2);
		REQUIRE(ndarray_opaque_frame_compare(
			static_cast<const void*>(raw1.data()),
			static_cast<const void*>(raw2.data()),
			frm
		));
	}
}
