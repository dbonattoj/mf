#include <catch.hpp>
#include <stdexcept>
#include "../src/ndarray/ndarray_timed_ring.h"
#include "support/ndarray.h"

using namespace mf;


TEST_CASE("ndarray_timed_ring", "[ndarray_timed_ring]") {
	ndsize<2> shape{320, 240};
	std::size_t duration = 10;
	
	ndarray_timed_ring<2, int> ring(shape, duration);
	
	REQUIRE(ring.current_time() == -1);
	REQUIRE(ring.readable_duration() == 0);
	REQUIRE(ring.writable_time_span() == time_span(0, 10));
	
	SECTION("single read/write") {
		// write frames [0,3[
		auto w_section(ring.begin_write_span(time_span(0, 3)));
		REQUIRE(w_section.shape().front() == 3);
		REQUIRE(ring.current_time() == -1);
		REQUIRE(ring.readable_duration() == 0);
		REQUIRE(ring.writable_time_span() == time_span(0, 10));
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);		
		ring.end_write(3);
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);
		
		// read frames [0,2[
		auto r_section(ring.begin_read_span(time_span(0, 2)));
		REQUIRE(ring.current_time() == 2);
		REQUIRE(ring.readable_time_span() == time_span(0, 3));
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_time_span() == time_span(3, 10));
		REQUIRE(ring.writable_duration() == 7);
		REQUIRE(r_section.shape().front() == 2);
		REQUIRE(r_section[0] == make_frame(shape, 0));
		REQUIRE(r_section[1] == make_frame(shape, 1));
		ring.end_read(2);
		REQUIRE(ring.current_time() == 2);
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

		// read frames ]2,4]
		auto r_section(ring.begin_read_span(time_span(2, 4)));
		REQUIRE(r_section[0] == make_frame(shape, 2));
		REQUIRE(r_section[1] == make_frame(shape, 3));	
		ring.end_read(2);
		REQUIRE(ring.current_time() == 4);
		
		// try to write [10,11[ (after current time)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(10,11)), sequencing_error);
		REQUIRE(ring.current_time() == 4);
		
		// try to write [3,10[ (before current time)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(3,10)), sequencing_error);
		REQUIRE(ring.current_time() == 4);
		
		// write [5,7[
		w_section.reset(ring.begin_write_span(time_span(5, 7)));
		w_section[0] = make_frame(shape, 5);
		w_section[1] = make_frame(shape, 6);
		ring.end_write(2);
		REQUIRE(ring.current_time() == 6);
		
		// try to read/skip [5,8[ (more than readable)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(5, 8)), sequencing_error);
		REQUIRE_THROWS_AS(ring.skip_span(time_span(5, 8)), sequencing_error);
		REQUIRE(ring.current_time() == 6);
		
		// try to read [5,16[ (more than capacity)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(5, 16)), std::invalid_argument);
		REQUIRE(ring.current_time() == 6);
		
		// read [5,7[
		r_section.reset(ring.begin_read_span(time_span(5, 7)));
		REQUIRE(r_section[0] == make_frame(shape, 5));
		REQUIRE(r_section[1] == make_frame(shape, 6));	
		ring.end_read(2);
		REQUIRE(ring.current_time() == 6);
	
		// try to write [7,18[ (more than capacity)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(7, 18)), std::invalid_argument);
		
		// write [7,11[ (out of [7,15[)
		w_section.reset(ring.begin_write_span(time_span(7, 15)));
		w_section[0] = make_frame(shape, 7);
		w_section[1] = make_frame(shape, 8);
		w_section[2] = make_frame(shape, 9);
		w_section[3] = make_frame(shape, 10);
		w_section[4] = make_frame(shape, 11);
		ring.end_write(5);
		REQUIRE(ring.current_time() == 11);
		
		// try to write [12,18[ (more than writable)
		REQUIRE_THROWS_AS(ring.begin_write_span(time_span(12, 18)), sequencing_error);
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(7,12));
		
		// skip [5,7[ (already passed)
		ring.skip_span(time_span(5, 7));
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(7,12));
		
		// skip [5,8[ (partially passed)
		ring.skip_span(time_span(5, 8));
		REQUIRE(ring.current_time() == 11);
		REQUIRE(ring.readable_time_span() == time_span(8,12));
		
		// skip [8,11[ (all of buffer)
		ring.skip_span(time_span(8, 12));
		REQUIRE(ring.current_time() == 11);	
	}
}
