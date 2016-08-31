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
#include <array>
#include <mf/nd/opaque_format/opaque_multi_ndarray_format.h>
#include <mf/nd/opaque_format/opaque_ndarray_format.h>

using namespace mf;

using unaligned_int32 = std::array<byte, 4>;
using unaligned_int64 = std::array<byte, 8>;

unaligned_int32 u32(std::int32_t i) { return *reinterpret_cast<unaligned_int32*>(&i); };
unaligned_int64 u64(std::int64_t i) { return *reinterpret_cast<unaligned_int64*>(&i); };


TEST_CASE("opaque_ndarray_format", "[nd][opaque_ndarray_format]") {
	ndarray_format afrm = make_ndarray_format<int>(100);
	opaque_ndarray_format frm(afrm);
		
	REQUIRE(frm.frame_size() == 100 * sizeof(int));
	REQUIRE(frm.frame_alignment_requirement() == alignof(int));
	REQUIRE(frm.has_array_format());
	REQUIRE(frm.array_format() == afrm);
	REQUIRE(frm.is_contiguous());
	REQUIRE(frm.is_pod());
	REQUIRE_FALSE(frm.has_parts());
			
	std::vector<int> raw1(100), raw2(100);
	for(std::ptrdiff_t i = 0; i < 100; ++i) {
		raw1[i] = i;
		raw2[i] = 2*i + 1;
	}

	REQUIRE(frm.compare_frame(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw1.data())
	));
	REQUIRE_FALSE(frm.compare_frame(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw2.data())
	));
	frm.copy_frame(
		static_cast<void*>(&raw2[0]),
		static_cast<const void*>(raw1.data())
	);
	REQUIRE(raw1 == raw2);
	REQUIRE(frm.compare_frame(
		static_cast<const void*>(raw1.data()),
		static_cast<const void*>(raw2.data())
	));
}


TEST_CASE("opaque_multi_ndarray_format", "[nd][opaque_multi_ndarray_format]") {
	opaque_multi_ndarray_format frm;
	ndarray_format afrm1 = make_ndarray_format<std::int32_t>(49);
	ndarray_format afrm2 = make_ndarray_format<std::int64_t>(20, 2*8);
	ndarray_format afrm3 = make_ndarray_format<std::int8_t>(3);

	REQUIRE(sizeof(std::int32_t) == alignof(std::int32_t));
	REQUIRE(sizeof(std::int64_t) == alignof(std::int64_t));
	
	frm.add_part(afrm1);
	frm.add_part(afrm2);
	frm.add_part(afrm3);
	
	REQUIRE(frm.has_parts());
	REQUIRE(frm.parts_count() == 3);
	REQUIRE_FALSE(frm.has_array_format());
	
	REQUIRE(frm.part_at(0).format == afrm1);
	REQUIRE(frm.part_at(0).offset == 0);
	REQUIRE(frm.part_at(1).format == afrm2);
	REQUIRE(frm.part_at(1).offset == 49*4 + 4);
	REQUIRE(frm.part_at(2).format == afrm3);
	REQUIRE(frm.part_at(2).offset == (49*4 + 4) + 20*2*8);
	REQUIRE(frm.frame_size() == (49*4 + 4) + (20*2*8) + 8);
	REQUIRE(is_multiple_of(frm.frame_size(), frm.frame_alignment_requirement()));
	REQUIRE(frm.frame_alignment_requirement() == 8);
	
	REQUIRE(frm.extract_part(0).offset == frm.part_at(0).offset);
	REQUIRE(frm.extract_part(0).format);
	REQUIRE(frm.extract_part(0).format->array_format() == frm.part_at(0).format);
	REQUIRE(frm.extract_part(1).offset == frm.part_at(1).offset);
	REQUIRE(frm.extract_part(1).format);
	REQUIRE(frm.extract_part(1).format->array_format() == frm.part_at(1).format);
	REQUIRE(frm.extract_part(2).offset == frm.part_at(2).offset);
	REQUIRE(frm.extract_part(2).format);
	REQUIRE(frm.extract_part(2).format->array_format() == frm.part_at(2).format);

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
	
	REQUIRE(frm.compare_frame(
		static_cast<const void*>(&frame1),
		static_cast<const void*>(&frame1)
	));
	REQUIRE_FALSE(frm.compare_frame(
		static_cast<const void*>(&frame1),
		static_cast<const void*>(&frame2)
	));
	frm.copy_frame(
		static_cast<void*>(&frame2),
		static_cast<const void*>(&frame1)
	);

	REQUIRE(frame1 == frame2);	
	REQUIRE(frame2.pad1[0] == 7);
	REQUIRE(frame2.part2[0].pad[0] == 7);
	REQUIRE(frame2.pad2[0] == 7);
	
	REQUIRE(frm.compare_frame(
		static_cast<const void*>(&frame1),
		static_cast<const void*>(&frame2)
	));
}
