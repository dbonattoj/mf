#include <catch.hpp>
#include "../src/util/memory.h"
#include "../src/ndarray_ring.h"

using namespace mf;

ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	ndarray<2, int> frame(shape);
	for(int& v : frame) v = i;
	return frame;
}


TEST_CASE("ndarray_ring", "[ndarray_ring]") {
	// choosing buffer duration&frame_sz such that is does not fit on page boundaries
	ndsize<2> shape(321, 240);
	std::size_t duration = 14;
	REQUIRE((shape.product() * duration * sizeof(int)) % system_page_size() != 0);
	
	ndarray_ring<2, int> ring(shape, duration);
	REQUIRE(ring.padding().front() != 0);
	REQUIRE(ring.padding()[1] == 0);
	REQUIRE(ring.padding()[2] == 0);
	
	REQUIRE(ring.total_duration() == duration);
	REQUIRE(ring.writable_duration() == duration);
	REQUIRE(ring.readable_duration() == 0);
	
	REQUIRE_THROWS(ring.write(ring.writable_duration() + 1, [&](const auto& section) { return 0; }));
	REQUIRE_THROWS(ring.read(1, [&](const auto& section) { return 0; }));
	REQUIRE_THROWS(ring.skip(1));
	
	
	SECTION("single read/write") {
		ring.write(1, [&](const auto& section) {
			REQUIRE(section.shape().front() == 1);
			section[0] = make_frame(shape, 1);
			return 1;
		});
		
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration - 1);
		REQUIRE(ring.readable_duration() == 1);
				
		ring.read(1, [&](const auto& section) {
			REQUIRE(section.shape().front() == 1);
			REQUIRE(section[0] == make_frame(shape, 1));
			REQUIRE(section[0] == make_frame(shape, 2));
			return 1;
		});
		
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration);
		REQUIRE(ring.readable_duration() == 0);
	}
	
	
	SECTION("multi read/write, wrap") {
		for(std::ptrdiff_t loop = 0; loop < 3; ++loop) {
			ring.write(3, [&](const auto& section) {
				REQUIRE(section.shape().front() == 3);
				for(std::ptrdiff_t i = 0; i < 3; ++i)
					section[i] = make_frame(shape, i);
				return 3;
			});
		
			REQUIRE(ring.total_duration() == duration);
			REQUIRE(ring.writable_duration() == duration - 3);
			REQUIRE(ring.readable_duration() == 3);
		
			ring.write(7, [&](const auto& section) {
				REQUIRE(section.shape().front() == 7);
				for(std::ptrdiff_t i = 0; i < 7; ++i)
					section[i] = make_frame(shape, i + 3);
				return 7;
			});
		
			REQUIRE(ring.total_duration() == duration);
			REQUIRE(ring.writable_duration() == duration - 10);
			REQUIRE(ring.readable_duration() == 10);
		
			ring.read(6, [&](const auto& section) {
				REQUIRE(section.shape().front() == 6);
				for(std::ptrdiff_t i = 0; i < 6; ++i)
					REQUIRE(section[i] == make_frame(shape, i));
				return 6;
			});
		
			REQUIRE(ring.total_duration() == duration);
			REQUIRE(ring.writable_duration() == duration - 4);
			REQUIRE(ring.readable_duration() == 4);

			ring.skip(2);
			
			REQUIRE(ring.total_duration() == duration);
			REQUIRE(ring.writable_duration() == duration - 2);
			REQUIRE(ring.readable_duration() == 2);	

			ring.read(2, [&](const auto& section) {
				REQUIRE(section.shape().front() == 2);
				for(std::ptrdiff_t i = 0; i < 2; ++i)
					REQUIRE(section[i] == make_frame(shape, i + 8));
				return 2;
			});
			
			REQUIRE(ring.total_duration() == duration);
			REQUIRE(ring.writable_duration() == duration);
			REQUIRE(ring.readable_duration() == 0);	
		}
	}
	
	
	SECTION("incomplete read/write") {
		// write frames 0, 1, 2
		ring.write(5, [&](const auto& section) {
			REQUIRE(section.shape().front() == 5);
			for(std::ptrdiff_t i = 0; i < 3; ++i)
				section[i] = make_frame(shape, i);
			return 3;
		});
	
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration - 3);
		REQUIRE(ring.readable_duration() == 3);
	
		// write frames 3, 4, 5
		ring.write(7, [&](const auto& section) {
			REQUIRE(section.shape().front() == 7);
			for(std::ptrdiff_t i = 0; i < 3; ++i)
				section[i] = make_frame(shape, i + 3);
			return 3;
		});
	
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration - 6);
		REQUIRE(ring.readable_duration() == 6);
	
		// read frames 0, 1, peek 2, 3, 4, 5
		ring.read(6, [&](const auto& section) {
			REQUIRE(section.shape().front() == 6);
			for(std::ptrdiff_t i = 0; i < 6; ++i)
				REQUIRE(section[i] == make_frame(shape, i));
			return 2;
		});
	
		REQUIRE(ring.total_duration() == duration);
		REQUIRE(ring.writable_duration() == duration - 4);
		REQUIRE(ring.readable_duration() == 4);

		// read frames 2, 3, 4, 5
		ring.read(4, [&](const auto& section) {
			REQUIRE(section.shape().front() == 4);
			for(std::ptrdiff_t i = 0; i < 4; ++i)
				REQUIRE(section[i] == make_frame(shape, i + 2));
			return 4;
		});
	}
}
