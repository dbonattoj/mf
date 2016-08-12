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
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFrngEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <catch.hpp>
#include <algorithm>
#include <cstdint>
#include <mf/os/memory.h>
#include <mf/queue/ring.h>
#include "../support/ndarray_opaque.h"

using namespace mf;
using namespace mf::test;

TEST_CASE("rng", "[queue][rng]") {
	// choosing buffer capacity&frame_sz such that is does not fit on page boundaries
	auto frm = opaque_frame_format();
	std::size_t capacity = 14;
	REQUIRE_FALSE(is_multiple_of(capacity * frm.frame_size(), system_page_size()));
	ring rng(frm, capacity);
	
	REQUIRE(rng.writable_duration() == capacity);
	REQUIRE(rng.readable_duration() == 0);
	
	REQUIRE_THROWS(rng.begin_write(rng.writable_duration() + 1));
	REQUIRE_THROWS(rng.begin_read(1));
	REQUIRE_THROWS(rng.skip(1));
	
	
	SECTION("single read/write") {
		auto w_section(rng.begin_write(1));
		REQUIRE(w_section.shape().front() == 1);
		w_section[0] = make_opaque_frame(1);
		rng.end_write(1);
			
		REQUIRE(rng.writable_duration() == capacity - 1);
		REQUIRE(rng.readable_duration() == 1);
			
		auto r_section(rng.begin_read(1));
		REQUIRE(r_section.shape().front() == 1);
		REQUIRE(r_section[0] == make_opaque_frame(1));
		rng.end_read(1);
		
		REQUIRE(rng.writable_duration() == capacity);
		REQUIRE(rng.readable_duration() == 0);
	}
	
	
	SECTION("multiple read/write, wrap") {
		for(std::ptrdiff_t loop = 0; loop < 3; ++loop) {
			INFO("loop " << loop);
			
			// write frames 0, 1, 2
			auto w_section(rng.begin_write(3));
			REQUIRE(w_section.shape().front() == 3);
			w_section[0] = make_opaque_frame(0);
			w_section[1] = make_opaque_frame(1);
			w_section[2] = make_opaque_frame(2);
			rng.end_write(3);
			REQUIRE(rng.writable_duration() == 11);
			REQUIRE(rng.readable_duration() == 3);
		
			// write frames 3, 4, 5, 6, 7, 8, 9
			w_section.reset(rng.begin_write(7));
			REQUIRE(w_section.shape().front() == 7);
			REQUIRE(rng.writable_duration() == 11); // no change yet...
			REQUIRE(rng.readable_duration() == 3);
			w_section[0] = make_opaque_frame(3);
			w_section[1] = make_opaque_frame(4);
			w_section[2] = make_opaque_frame(5);
			w_section[3] = make_opaque_frame(6);
			w_section[4] = make_opaque_frame(7);
			w_section[5] = make_opaque_frame(8);
			w_section[6] = make_opaque_frame(9);
			REQUIRE_THROWS(rng.end_write(12)); // report too large...
			REQUIRE(rng.writable_duration() == 11); // still no change
			REQUIRE(rng.readable_duration() == 3);
			rng.end_write(7);		
			REQUIRE(rng.writable_duration() == 4);
			REQUIRE(rng.readable_duration() == 10);
			
			// try to read 11 frames
			REQUIRE_THROWS(rng.begin_read(11));
		
			// read frames 0, 1, 2, 3, 4, 5
			auto r_section(rng.begin_read(6));
			REQUIRE(rng.writable_duration() == 4);
			REQUIRE(rng.readable_duration() == 10);
			REQUIRE(r_section.shape().front() == 6);
			REQUIRE(r_section[0] == make_opaque_frame(0));
			REQUIRE(r_section[1] == make_opaque_frame(1));
			REQUIRE(r_section[2] == make_opaque_frame(2));
			REQUIRE(r_section[3] == make_opaque_frame(3));
			REQUIRE(r_section[4] == make_opaque_frame(4));
			REQUIRE(r_section[5] == make_opaque_frame(5));
			REQUIRE_THROWS(rng.end_read(11));
			REQUIRE(rng.writable_duration() == 4);
			REQUIRE(rng.readable_duration() == 10);
			rng.end_read(6);	
			REQUIRE(rng.writable_duration() == 10);
			REQUIRE(rng.readable_duration() == 4);

			// try to skip 6, 7, 8, 9, 10!, 11!
			REQUIRE_THROWS(rng.skip(6));
			REQUIRE(rng.writable_duration() == 10);
			REQUIRE(rng.readable_duration() == 4);

			// skip frames 6, 7
			rng.skip(2);			
			REQUIRE(rng.writable_duration() == 12);
			REQUIRE(rng.readable_duration() == 2);	

			// try to write 13 frames
			REQUIRE_THROWS(rng.begin_write(13));
			REQUIRE(rng.writable_duration() == 12);
			REQUIRE(rng.readable_duration() == 2);				

			// read frames 8, 9
			r_section.reset(rng.begin_read(2));
			REQUIRE(r_section.shape().front() == 2);
			REQUIRE(r_section[0] == make_opaque_frame(8));
			REQUIRE(r_section[1] == make_opaque_frame(9));
			rng.end_read(2);
			REQUIRE(rng.writable_duration() == 14);
			REQUIRE(rng.readable_duration() == 0);	
		}
	}

		
	SECTION("incomplete read/write") {
		// write frames 0, 1, 2
		auto w_section(rng.begin_write(5));
		REQUIRE(w_section.shape().front() == 5);
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);
		rng.end_write(3);
		REQUIRE(rng.writable_duration() == 11);
		REQUIRE(rng.readable_duration() == 3);
	
		// write frames 3, 4, 5
		w_section.reset(rng.begin_write(7));
		REQUIRE(w_section.shape().front() == 7);
		w_section[0] = make_opaque_frame(3);
		w_section[1] = make_opaque_frame(4);
		w_section[2] = make_opaque_frame(5);
		rng.end_write(3);
		REQUIRE(rng.writable_duration() == 8);
		REQUIRE(rng.readable_duration() == 6);
		
		// read frames 0, 1, peek 2, 3, 4, 5
		auto r_section(rng.begin_read(6));
		REQUIRE(r_section.shape().front() == 6);
		REQUIRE(r_section[0] == make_opaque_frame(0));
		REQUIRE(r_section[1] == make_opaque_frame(1));
		REQUIRE(r_section[2] == make_opaque_frame(2));
		REQUIRE(r_section[3] == make_opaque_frame(3));
		REQUIRE(r_section[4] == make_opaque_frame(4));
		REQUIRE(r_section[5] == make_opaque_frame(5));
		rng.end_read(2);
		REQUIRE(rng.writable_duration() == 10);
		REQUIRE(rng.readable_duration() == 4);

		// read frames 2, 3, 4, 5 (again)
		r_section.reset(rng.begin_read(4));
		REQUIRE(r_section.shape().front() == 4);
		REQUIRE(r_section[0] == make_opaque_frame(2));
		REQUIRE(r_section[1] == make_opaque_frame(3));
		REQUIRE(r_section[2] == make_opaque_frame(4));
		REQUIRE(r_section[3] == make_opaque_frame(5));
		rng.end_read(4);
		REQUIRE(rng.writable_duration() == 14);
		REQUIRE(rng.readable_duration() == 0);
	}
	
	
	SECTION("zero read/write") {
		// write frames 0, 1, 2
		auto w_section(rng.begin_write(3));
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);
		rng.end_write(3);
		REQUIRE(rng.writable_duration() == 11);
		REQUIRE(rng.readable_duration() == 3);

		// read none, out of 0, 1
		auto r_section(rng.begin_read(2));
		rng.end_read(0);
		REQUIRE(rng.writable_duration() == 11);
		REQUIRE(rng.readable_duration() == 3);

		// write none, out of 3, 4
		w_section.reset(rng.begin_write(2));
		rng.end_write(0);
		REQUIRE(rng.writable_duration() == 11);
		REQUIRE(rng.readable_duration() == 3);
		
		// skip none
		rng.skip(0);
		REQUIRE(rng.writable_duration() == 11);
		REQUIRE(rng.readable_duration() == 3);
	}
}
