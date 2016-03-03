#include <catch.hpp>
#include "../src/util/memory.h"
#include "../src/ndarray_ring.h"

#include <iostream>

using namespace mf;

ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	ndarray<2, int> frame(shape);
	for(int& v : frame) v = i;
	return frame;
}


TEST_CASE("ndarray_ring", "[ndarray_ring]") {
	// choosing buffer duration&frame_sz such that is does not fit on page boundaries
	ndsize<2> shape(321, 240);
	std::size_t duration = 10;
	REQUIRE((shape.product() * duration * sizeof(int)) % system_page_size() != 0);
	
	ndarray_ring<2, int> ring(shape, duration);
	REQUIRE(ring.padding().front() != 0);
	REQUIRE(ring.padding()[1] == 0);
	REQUIRE(ring.padding()[2] == 0);
	
	REQUIRE(ring.total_duration() == duration);
	REQUIRE(ring.writable_duration() == duration);
	REQUIRE(ring.readable_duration() == 0);
	
	SECTION("FILO single read/write") {
		ring.write(1, [&](const auto& section) {
			REQUIRE(section.shape().front() == 1);
			section[0] = make_frame(shape, 1);
		});
		
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration - 1);
		REQUIRE(ring.readable_duration() == 1);
		
		ring.read(1, [&](const auto& section) {
			REQUIRE(section.shape().front() == 1);
			REQUIRE(section[0] == make_frame(shape, 1));
			REQUIRE(section[0] == make_frame(shape, 2));
		});
		
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration);
		REQUIRE(ring.readable_duration() == 0);
	}
}
