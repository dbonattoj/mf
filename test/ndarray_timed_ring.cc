#include <catch.hpp>
#include <stdexcept>
#include "../src/queue/ndarray_ring.h"
#include "support/ndarray.h"

using namespace mf;
using namespace mf::test;


TEST_CASE("ndarray_timed_ring", "[ndarray_timed_ring]") {
	ndsize<2> shape{320, 240};
	std::size_t duration = 10;
	
	ndarray_timed_ring<2, int> ring(shape, duration);
	
	REQUIRE(ring.current_time() == -1);
	REQUIRE(ring.read_start_time() == 0);
	REQUIRE(ring.write_start_time() == 0);
	REQUIRE(ring.readable_duration() == 0);
	REQUIRE(ring.writable_time_span() == time_span(0, 10));
	
	SECTION("single read/write") {
		// write frames [0,3[
		auto w_section(ring.begin_write_span(time_span(0, 3)));
		REQUIRE(w_section.shape().front() == 3);
		REQUIRE(ring.current_time() == -1);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 0);
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(0, 10));
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);		
		ring.end_write(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);
		
		// read frames [0,2[
		auto r_section(ring.begin_read_span(time_span(0, 2)));
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);
		REQUIRE(r_section.shape().front() == 2);
		compare_frames(shape, r_section, {0, 1});
		ring.end_read(2);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 2);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(2, 3));
		REQUIRE(ring.readable_duration() == 1);
		REQUIRE(ring.writable_time_span() == time_span(3, 12));
		REQUIRE(ring.writable_duration() == 9);		
	}
		
	SECTION("multiple read/write") {	
		// write frames [0,5[
		auto w_section(ring.begin_write_span(time_span(0, 5)));
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);		
		w_section[3] = make_frame(shape, 3);		
		w_section[4] = make_frame(shape, 4);		
		ring.end_write(5);
		REQUIRE(ring.current_time() == 4);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 5);

		// read frames ]2,4]
		auto r_section(ring.begin_read_span(time_span(2, 4)));
		compare_frames(shape, r_section, {2, 3});
		ring.end_read(2);
		REQUIRE(ring.current_time() == 4);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 5);

		
		// try to write [10,11[ (after current time)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(10,11)), sequencing_error);
		REQUIRE(ring.current_time() == 4);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 5);
		
		// try to write [3,10[ (before current time)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(3,10)), sequencing_error);
		REQUIRE(ring.current_time() == 4);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 5);
		
		// write [5,7[
		w_section.reset(ring.begin_write_span(time_span(5, 7)));
		w_section[0] = make_frame(shape, 5);
		w_section[1] = make_frame(shape, 6);
		ring.end_write(2);
		REQUIRE(ring.current_time() == 6);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 7);
		
		// try to read/skip [5,8[ (more than readable)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(5, 8)), sequencing_error);
		REQUIRE_THROWS_AS(ring.skip_span(time_span(5, 8)), sequencing_error);
		REQUIRE(ring.current_time() == 6);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 7);
		
		// try to read [5,16[ (more than capacity)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(5, 16)), std::invalid_argument);
		REQUIRE(ring.current_time() == 6);
		REQUIRE(ring.read_start_time() == 4);
		REQUIRE(ring.write_start_time() == 7);
		
		// read [5,7[
		r_section.reset(ring.begin_read_span(time_span(5, 7)));
		compare_frames(shape, r_section, {5, 6});
		ring.end_read(2);
		REQUIRE(ring.current_time() == 6);
		REQUIRE(ring.read_start_time() == 7);
		REQUIRE(ring.write_start_time() == 7);
	
		// try to write [7,18[ (more than capacity)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(7, 18)), std::invalid_argument);
		REQUIRE(ring.current_time() == 6);
		REQUIRE(ring.read_start_time() == 7);
		REQUIRE(ring.write_start_time() == 7);
		
		// write [7,12[ (out of [7,15[)
		w_section.reset(ring.begin_write_span(time_span(7, 15)));
		w_section[0] = make_frame(shape, 7);
		w_section[1] = make_frame(shape, 8);
		w_section[2] = make_frame(shape, 9);
		w_section[3] = make_frame(shape, 10);
		w_section[4] = make_frame(shape, 11);
		ring.end_write(5);
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.read_start_time() == 7);
		REQUIRE(ring.write_start_time() == 12);
		
		// try to write [12,18[ (more than writable)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(12, 18)), sequencing_error);
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(7,12));
		REQUIRE(ring.read_start_time() == 7);
		REQUIRE(ring.write_start_time() == 12);
		
		// skip [5,7[ (already passed)
		ring.skip_span(time_span(5, 7));
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(7,12));
		REQUIRE(ring.read_start_time() == 7);
		REQUIRE(ring.write_start_time() == 12);
		
		// skip [5,8[ (partially passed)
		ring.skip_span(time_span(5, 8));
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(8,12));
		REQUIRE(ring.read_start_time() == 8);
		REQUIRE(ring.write_start_time() == 12);
		
		// skip [8,11[ (all of buffer)
		ring.skip_span(time_span(8, 12));
		REQUIRE(ring.current_time() == 11);	
		REQUIRE(ring.read_start_time() == 12);
		REQUIRE(ring.write_start_time() == 12);
	}
	
	
	SECTION("seek") {
		// write frames [0, 3[
		auto w_section(ring.begin_write(3));
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);
		ring.end_write(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);

		// seek to 1 (--> short seek)
		ring.seek(1);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 1);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(1, 3));
		REQUIRE(ring.readable_duration() == 2);
		REQUIRE(ring.writable_time_span() == time_span(3, 11));
		REQUIRE(ring.writable_duration() == 8);
		
		// seek to 0 (--> long seek)
		ring.seek(0);
		REQUIRE(ring.current_time() == -1);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 0);
		REQUIRE(ring.readable_time_span() == time_span(0, 0));
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(0, 10));
		REQUIRE(ring.writable_duration() == 10);

		// seek to 3 (--> long seek)
		ring.seek(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 3);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(3, 3));
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(3, 13));
		REQUIRE(ring.writable_duration() == 10);
		
		// seek to 3 (--> noop)
		ring.seek(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 3);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(3, 3));
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(3, 13));
		REQUIRE(ring.writable_duration() == 10);

		// seek to 30 (--> long seek)
		ring.seek(30);
		REQUIRE(ring.current_time() == 29);
		REQUIRE(ring.read_start_time() == 30);
		REQUIRE(ring.write_start_time() == 30);
		REQUIRE(ring.readable_time_span() == time_span(30, 30));
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(30, 40));
		REQUIRE(ring.writable_duration() == 10);

		// seek to 1 (--> long seek)
		ring.seek(1);
		REQUIRE(ring.current_time() == 0);
		REQUIRE(ring.read_start_time() == 1);
		REQUIRE(ring.write_start_time() == 1);
		REQUIRE(ring.readable_time_span() == time_span(1, 1));
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(1, 11));
		REQUIRE(ring.writable_duration() == 10);
	}
	
	
	SECTION("zero read/write") {
		// write frames [0, 3[
		auto w_section(ring.begin_write(3));
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);
		ring.end_write(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);

		// read none, out of [0, 2[
		auto r_section(ring.begin_read_span(time_span(0, 2)));
		ring.end_read(0);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);


		// write none, out of [3, 5[
		w_section.reset(ring.begin_write(2));
		ring.end_write(0);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 0);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);
		
		// read none, out of [1, 3[, skips 1
		r_section.reset(ring.begin_read_span(time_span(1, 3)));
		ring.end_write(0);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.read_start_time() == 1);
		REQUIRE(ring.write_start_time() == 3);
		REQUIRE(ring.readable_time_span() == time_span(1, 3));
		REQUIRE(ring.readable_duration() == 2);
		REQUIRE(ring.writable_time_span() == time_span(3, 11));
		REQUIRE(ring.writable_duration() == 8);
	}
}
