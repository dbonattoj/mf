#include <catch.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include "../src/ndarray_shared_ring.h"
#include <iostream>

using namespace mf;

static ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	ndarray<2, int> frame(shape);
	std::fill(frame.begin(), frame.end(), i);
	return frame;
}

TEST_CASE("ndarray_shared_ring", "[ndarray_shared_ring]") {
	ndsize<2> shape(320, 240);
	std::size_t duration = 10;
	
	ndarray_shared_ring<2, int> ring(shape, duration);

	SECTION("background writer") {
		// secondary thread: keeps writing frames into buffer
		std::atomic_bool running(true);
		std::thread writer([&]() {
			using namespace std::literals;
			while(running) {
				std::this_thread::sleep_for(50ms);
				auto w_section = ring.begin_write(1);
				w_section[0] = make_frame(shape, ring.current_time()+1);
				ring.end_write(1);
			}
		});
		
		// read frames 0, 1, 2: wait until they become available
		auto r_section = ring.begin_read(3);
		REQUIRE(r_section[0] == make_frame(shape, 0));
		REQUIRE(r_section[1] == make_frame(shape, 1));
		REQUIRE(r_section[2] == make_frame(shape, 2));
		ring.end_read(3);
		
		// end secondary thread
		running = false;
		writer.join();
	}
}