#include <catch.hpp>
#include <algorithm>
#include "../src/utility/memory.h"
#include "../src/ndarray/ndarray_ring.h"
#include "support/ndarray.h"

using namespace mf;
using namespace mf::test;

TEST_CASE("ndarray_ring", "[ndarray_ring]") {
	// choosing buffer duration&frame_sz such that is does not fit on page boundaries
	ndsize<2> shape{321, 240};
	std::size_t duration = 14;
	REQUIRE((shape.product() * duration * sizeof(int)) % system_page_size() != 0);
	
	ndarray_ring<2, int> ring(shape, duration);
	REQUIRE(ring.padding().front() != 0);
	REQUIRE(ring.padding()[1] == 0);
	REQUIRE(ring.padding()[2] == 0);
	
	REQUIRE(ring.writable_duration() == duration);
	REQUIRE(ring.readable_duration() == 0);
	
	REQUIRE_THROWS(ring.begin_write(ring.writable_duration() + 1));
	REQUIRE_THROWS(ring.begin_read(1));
	REQUIRE_THROWS(ring.skip(1));
	
	
	SECTION("single read/write") {
		auto w_section(ring.begin_write(1));
		REQUIRE(w_section.shape().front() == 1);
		w_section[0] = make_frame(shape, 1);
		ring.end_write(1);
			
		REQUIRE(ring.writable_duration() == duration - 1);
		REQUIRE(ring.readable_duration() == 1);
			
		auto r_section(ring.begin_read(1));
		REQUIRE(r_section.shape().front() == 1);
		REQUIRE(r_section[0] == make_frame(shape, 1));
		ring.end_read(1);
		
		REQUIRE(ring.writable_duration() == duration);
		REQUIRE(ring.readable_duration() == 0);
	}
	
	
	SECTION("multiple read/write, wrap") {
		for(std::ptrdiff_t loop = 0; loop < 3; ++loop) {
			// write frames 0, 1, 2
			auto w_section(ring.begin_write(3));
			REQUIRE(w_section.shape().front() == 3);
			w_section[0] = make_frame(shape, 0);
			w_section[1] = make_frame(shape, 1);
			w_section[2] = make_frame(shape, 2);
			ring.end_write(3);
			REQUIRE(ring.writable_duration() == 11);
			REQUIRE(ring.readable_duration() == 3);
		
			// write frames 3, 4, 5, 6, 7, 8, 9
			w_section.reset(ring.begin_write(7));
			REQUIRE(w_section.shape().front() == 7);
			REQUIRE(ring.writable_duration() == 11); // no change yet...
			REQUIRE(ring.readable_duration() == 3);
			w_section[0] = make_frame(shape, 3);
			w_section[1] = make_frame(shape, 4);
			w_section[2] = make_frame(shape, 5);
			w_section[3] = make_frame(shape, 6);
			w_section[4] = make_frame(shape, 7);
			w_section[5] = make_frame(shape, 8);
			w_section[6] = make_frame(shape, 9);
			REQUIRE_THROWS(ring.end_write(12)); // report too large...
			REQUIRE(ring.writable_duration() == 11); // still no change
			REQUIRE(ring.readable_duration() == 3);
			ring.end_write(7);		
			REQUIRE(ring.writable_duration() == 4);
			REQUIRE(ring.readable_duration() == 10);
			
			// try to read 11 frames
			REQUIRE_THROWS(ring.begin_read(11));
		
			// read frames 0, 1, 2, 3, 4, 5
			auto r_section(ring.begin_read(6));
			REQUIRE(ring.writable_duration() == 4);
			REQUIRE(ring.readable_duration() == 10);
			REQUIRE(r_section.shape().front() == 6);
			REQUIRE(r_section[0] == make_frame(shape, 0));
			REQUIRE(r_section[1] == make_frame(shape, 1));
			REQUIRE(r_section[2] == make_frame(shape, 2));
			REQUIRE(r_section[3] == make_frame(shape, 3));
			REQUIRE(r_section[4] == make_frame(shape, 4));
			REQUIRE(r_section[5] == make_frame(shape, 5));
			REQUIRE_THROWS(ring.end_read(11));
			REQUIRE(ring.writable_duration() == 4);
			REQUIRE(ring.readable_duration() == 10);
			ring.end_read(6);	
			REQUIRE(ring.writable_duration() == 10);
			REQUIRE(ring.readable_duration() == 4);

			// try to skip 6, 7, 8, 9, 10!, 11!
			REQUIRE_THROWS(ring.skip(6));
			REQUIRE(ring.writable_duration() == 10);
			REQUIRE(ring.readable_duration() == 4);

			// skip frames 6, 7
			ring.skip(2);			
			REQUIRE(ring.writable_duration() == 12);
			REQUIRE(ring.readable_duration() == 2);	

			// try to write 13 frames
			REQUIRE_THROWS(ring.begin_write(13));
			REQUIRE(ring.writable_duration() == 12);
			REQUIRE(ring.readable_duration() == 2);				

			// read frames 8, 9
			r_section.reset(ring.begin_read(2));
			REQUIRE(r_section.shape().front() == 2);
			REQUIRE(r_section[0] == make_frame(shape, 8));
			REQUIRE(r_section[1] == make_frame(shape, 9));
			ring.end_read(2);
			REQUIRE(ring.writable_duration() == 14);
			REQUIRE(ring.readable_duration() == 0);	
		}
	}

		
	SECTION("incomplete read/write") {
		// write frames 0, 1, 2
		auto w_section(ring.begin_write(5));
		REQUIRE(w_section.shape().front() == 5);
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);
		ring.end_write(3);
		REQUIRE(ring.writable_duration() == 11);
		REQUIRE(ring.readable_duration() == 3);
	
		// write frames 3, 4, 5
		w_section.reset(ring.begin_write(7));
		REQUIRE(w_section.shape().front() == 7);
		w_section[0] = make_frame(shape, 3);
		w_section[1] = make_frame(shape, 4);
		w_section[2] = make_frame(shape, 5);
		ring.end_write(3);
		REQUIRE(ring.writable_duration() == 8);
		REQUIRE(ring.readable_duration() == 6);
		
		// read frames 0, 1, peek 2, 3, 4, 5
		auto r_section(ring.begin_read(6));
		REQUIRE(r_section.shape().front() == 6);
		REQUIRE(r_section[0] == make_frame(shape, 0));
		REQUIRE(r_section[1] == make_frame(shape, 1));
		REQUIRE(r_section[2] == make_frame(shape, 2));
		REQUIRE(r_section[3] == make_frame(shape, 3));
		REQUIRE(r_section[4] == make_frame(shape, 4));
		REQUIRE(r_section[5] == make_frame(shape, 5));
		ring.end_read(2);
		REQUIRE(ring.writable_duration() == 10);
		REQUIRE(ring.readable_duration() == 4);

		// read frames 2, 3, 4, 5 (again)
		r_section.reset(ring.begin_read(4));
		REQUIRE(r_section.shape().front() == 4);
		REQUIRE(r_section[0] == make_frame(shape, 2));
		REQUIRE(r_section[1] == make_frame(shape, 3));
		REQUIRE(r_section[2] == make_frame(shape, 4));
		REQUIRE(r_section[3] == make_frame(shape, 5));
		ring.end_read(4);
		REQUIRE(ring.writable_duration() == 14);
		REQUIRE(ring.readable_duration() == 0);
	}
}
