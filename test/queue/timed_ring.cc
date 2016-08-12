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
#include <stdexcept>
#include <mf/queue/timed_ring.h>
#include "../support/ndarray_opaque.h"

using namespace mf;
using namespace mf::test;


TEST_CASE("timed_ring", "[queue][timed_ring]") {
	auto frm = opaque_frame_format();
	std::size_t capacity = 10;
	REQUIRE_FALSE(is_multiple_of(capacity * frm.frame_size(), system_page_size()));
	timed_ring rng(frm, capacity);
	
	REQUIRE(rng.current_time() == -1);
	REQUIRE(rng.read_start_time() == 0);
	REQUIRE(rng.write_start_time() == 0);
	REQUIRE(rng.readable_duration() == 0);
	REQUIRE(rng.writable_time_span() == time_span(0, 10));
	
	SECTION("single read/write") {
		// write frames [0,3[
		auto w_section(rng.begin_write_span(time_span(0, 3)));
		REQUIRE(w_section.shape().front() == 3);
		REQUIRE(rng.current_time() == -1);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 0);
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(0, 10));
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);		
		rng.end_write(3);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);
		
		// read frames [0,2[
		auto r_section(rng.begin_read_span(time_span(0, 2)));
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);
		REQUIRE(r_section.shape().front() == 2);
		compare_opaque_frames(r_section, {0, 1});
		rng.end_read(2);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 2);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(2, 3));
		REQUIRE(rng.readable_duration() == 1);
		REQUIRE(rng.writable_time_span() == time_span(3, 12));
		REQUIRE(rng.writable_duration() == 9);		
	}
		
	SECTION("multiple read/write") {	
		// write frames [0,5[
		auto w_section(rng.begin_write_span(time_span(0, 5)));
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);		
		w_section[3] = make_opaque_frame(3);		
		w_section[4] = make_opaque_frame(4);		
		rng.end_write(5);
		REQUIRE(rng.current_time() == 4);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 5);

		// read frames ]2,4]
		auto r_section(rng.begin_read_span(time_span(2, 4)));
		compare_opaque_frames(r_section, {2, 3});
		rng.end_read(2);
		REQUIRE(rng.current_time() == 4);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 5);

		
		// try to write [10,11[ (after current time)
		REQUIRE_THROWS_AS(rng.begin_write_span(time_span(10,11)), sequencing_error);
		REQUIRE(rng.current_time() == 4);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 5);
		
		// try to write [3,10[ (before current time)
		REQUIRE_THROWS_AS(rng.begin_write_span(time_span(3,10)), sequencing_error);
		REQUIRE(rng.current_time() == 4);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 5);
		
		// write [5,7[
		w_section.reset(rng.begin_write_span(time_span(5, 7)));
		w_section[0] = make_opaque_frame(5);
		w_section[1] = make_opaque_frame(6);
		rng.end_write(2);
		REQUIRE(rng.current_time() == 6);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 7);
		
		// try to read/skip [5,8[ (more than readable)
		REQUIRE_THROWS_AS(rng.begin_read_span(time_span(5, 8)), sequencing_error);
		REQUIRE_THROWS_AS(rng.skip_span(time_span(5, 8)), sequencing_error);
		REQUIRE(rng.current_time() == 6);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 7);
		
		// try to read [5,16[ (more than capacity)
		REQUIRE_THROWS_AS(rng.begin_read_span(time_span(5, 16)), std::invalid_argument);
		REQUIRE(rng.current_time() == 6);
		REQUIRE(rng.read_start_time() == 4);
		REQUIRE(rng.write_start_time() == 7);
		
		// read [5,7[
		r_section.reset(rng.begin_read_span(time_span(5, 7)));
		compare_opaque_frames(r_section, {5, 6});
		rng.end_read(2);
		REQUIRE(rng.current_time() == 6);
		REQUIRE(rng.read_start_time() == 7);
		REQUIRE(rng.write_start_time() == 7);
	
		// try to write [7,18[ (more than capacity)
		REQUIRE_THROWS_AS(rng.begin_write_span(time_span(7, 18)), std::invalid_argument);
		REQUIRE(rng.current_time() == 6);
		REQUIRE(rng.read_start_time() == 7);
		REQUIRE(rng.write_start_time() == 7);
		
		// write [7,12[ (out of [7,15[)
		w_section.reset(rng.begin_write_span(time_span(7, 15)));
		w_section[0] = make_opaque_frame(7);
		w_section[1] = make_opaque_frame(8);
		w_section[2] = make_opaque_frame(9);
		w_section[3] = make_opaque_frame(10);
		w_section[4] = make_opaque_frame(11);
		rng.end_write(5);
		REQUIRE(rng.current_time() == 11);
		REQUIRE(rng.read_start_time() == 7);
		REQUIRE(rng.write_start_time() == 12);
		
		// try to write [12,18[ (more than writable)
		REQUIRE_THROWS_AS(rng.begin_write_span(time_span(12, 18)), sequencing_error);
		REQUIRE(rng.current_time() == 11);
		REQUIRE(rng.readable_time_span() == time_span(7,12));
		REQUIRE(rng.read_start_time() == 7);
		REQUIRE(rng.write_start_time() == 12);
		
		// skip [5,7[ (already passed)
		rng.skip_span(time_span(5, 7));
		REQUIRE(rng.current_time() == 11);
		REQUIRE(rng.readable_time_span() == time_span(7,12));
		REQUIRE(rng.read_start_time() == 7);
		REQUIRE(rng.write_start_time() == 12);
		
		// skip [5,8[ (partially passed)
		rng.skip_span(time_span(5, 8));
		REQUIRE(rng.current_time() == 11);
		REQUIRE(rng.readable_time_span() == time_span(8,12));
		REQUIRE(rng.read_start_time() == 8);
		REQUIRE(rng.write_start_time() == 12);
		
		// skip [8,11[ (all of buffer)
		rng.skip_span(time_span(8, 12));
		REQUIRE(rng.current_time() == 11);	
		REQUIRE(rng.read_start_time() == 12);
		REQUIRE(rng.write_start_time() == 12);
	}
	
	
	SECTION("seek") {
		// write frames [0, 3[
		auto w_section(rng.begin_write(3));
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);
		rng.end_write(3);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);

		// seek to 1 (--> short seek)
		rng.seek(1);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 1);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(1, 3));
		REQUIRE(rng.readable_duration() == 2);
		REQUIRE(rng.writable_time_span() == time_span(3, 11));
		REQUIRE(rng.writable_duration() == 8);
		
		// seek to 0 (--> long seek)
		rng.seek(0);
		REQUIRE(rng.current_time() == -1);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 0);
		REQUIRE(rng.readable_time_span() == time_span(0, 0));
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(0, 10));
		REQUIRE(rng.writable_duration() == 10);

		// seek to 3 (--> long seek)
		rng.seek(3);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 3);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(3, 3));
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(3, 13));
		REQUIRE(rng.writable_duration() == 10);
		
		// seek to 3 (--> noop)
		rng.seek(3);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 3);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(3, 3));
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(3, 13));
		REQUIRE(rng.writable_duration() == 10);

		// seek to 30 (--> long seek)
		rng.seek(30);
		REQUIRE(rng.current_time() == 29);
		REQUIRE(rng.read_start_time() == 30);
		REQUIRE(rng.write_start_time() == 30);
		REQUIRE(rng.readable_time_span() == time_span(30, 30));
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(30, 40));
		REQUIRE(rng.writable_duration() == 10);

		// seek to 1 (--> long seek)
		rng.seek(1);
		REQUIRE(rng.current_time() == 0);
		REQUIRE(rng.read_start_time() == 1);
		REQUIRE(rng.write_start_time() == 1);
		REQUIRE(rng.readable_time_span() == time_span(1, 1));
		REQUIRE(rng.readable_duration() == 0);
		REQUIRE(rng.writable_time_span() == time_span(1, 11));
		REQUIRE(rng.writable_duration() == 10);
	}
	
	
	SECTION("zero read/write") {
		// write frames [0, 3[
		auto w_section(rng.begin_write(3));
		w_section[0] = make_opaque_frame(0);
		w_section[1] = make_opaque_frame(1);
		w_section[2] = make_opaque_frame(2);
		rng.end_write(3);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);

		// read none, out of [0, 2[
		auto r_section(rng.begin_read_span(time_span(0, 2)));
		rng.end_read(0);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);


		// write none, out of [3, 5[
		w_section.reset(rng.begin_write(2));
		rng.end_write(0);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 0);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(0, 3));
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_time_span() == time_span(3, 10));
		REQUIRE(rng.writable_duration() == 7);
		
		// read none, out of [1, 3[, skips 1
		r_section.reset(rng.begin_read_span(time_span(1, 3)));
		rng.end_write(0);
		REQUIRE(rng.current_time() == 2);
		REQUIRE(rng.read_start_time() == 1);
		REQUIRE(rng.write_start_time() == 3);
		REQUIRE(rng.readable_time_span() == time_span(1, 3));
		REQUIRE(rng.readable_duration() == 2);
		REQUIRE(rng.writable_time_span() == time_span(3, 11));
		REQUIRE(rng.writable_duration() == 8);
	}
}
