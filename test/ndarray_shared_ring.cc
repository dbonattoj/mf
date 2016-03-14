#include <catch.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include "../src/ndarray_shared_ring.h"
#include <iostream>

using namespace mf;
using namespace std::literals;

static ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	ndarray<2, int> frame(shape);
	std::fill(frame.begin(), frame.end(), i);
	return frame;
}

TEST_CASE("ndarray_shared_ring", "[ndarray_shared_ring]") {
	ndsize<2> shape{320, 240};
	std::size_t duration = 5;
	
	ndarray_shared_ring<2, int> ring(shape, duration);

	SECTION("background writer") {
		// secondary thread: keeps writing frames into buffer
		std::atomic_bool running(true);
		std::thread writer([&]() {
			int t = 0;
			while(running) {
				std::this_thread::sleep_for(5ms);
				auto w_section = ring.begin_write(1);
				w_section[0] = make_frame(shape, t++);
				ring.end_write(1);
			}
		});
		
		// check that writer is continuously writing
		auto t0 = ring.current_time();
		std::this_thread::sleep_for(20ms);
		CHECK(ring.current_time() > t0);
		
		// read 3 frames (0, 1, 2)
		auto r_section = ring.begin_read(3);
		REQUIRE(r_section[0] == make_frame(shape, 0));
		REQUIRE(r_section[1] == make_frame(shape, 1));
		REQUIRE(r_section[2] == make_frame(shape, 2));
		ring.end_read(3);
		REQUIRE(ring.current_time() >= 2);
		
		// skip 2 frames (3, 4)
		ring.skip(2);
		REQUIRE(ring.current_time() >= 4);
		
		// try to read frames span (4, 5, 6) (already passed)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(4, 7)), sequencing_error);
		
		// read frames (5, 6)
		r_section.reset(ring.begin_read_span(time_span(5, 7)));
		REQUIRE(r_section[0] == make_frame(shape, 5));
		REQUIRE(r_section[1] == make_frame(shape, 6));		
		ring.end_read(2);
		REQUIRE(ring.current_time() >= 6);
		
		// read frames (20, 21)
		// skip previous
		r_section.reset(ring.begin_read_span(time_span(20, 22)));
		REQUIRE(r_section[0] == make_frame(shape, 20));
		REQUIRE(r_section[1] == make_frame(shape, 21));		
		ring.end_read(2);
		REQUIRE(ring.current_time() >= 21);
		
		// skip frames (24...40)
		// skip previous & more than ring capacity
		ring.skip_span(time_span(24, 41));
		REQUIRE(ring.current_time() >= 40);
		
		// end secondary thread
		running = false;
		writer.join();
	}

	SECTION("invalid use") {
		auto w_section = ring.begin_write(3);
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		w_section[2] = make_frame(shape, 2);
		ring.end_write(3);
		
		REQUIRE(ring.readable_duration() == 3);
		REQUIRE(ring.writable_duration() == 2);

		SECTION("concurrent reading") {
			std::mutex wait_;
			wait_.lock();
			std::thread other([&]() {
				wait_.lock();
				ring.begin_read(2);
				std::this_thread::sleep_for(10ms);
				ring.end_read(2);
			});
			
			wait_.unlock(); // let other thread read for 10ms now
			std::this_thread::sleep_for(5ms);
			// it should be reading now...
			CHECK_THROWS_AS(ring.begin_read(2), sequencing_error);
			
			other.join();
		}
		
		SECTION("concurrent writing") {
			std::mutex wait_;
			wait_.lock();
			std::thread other([&]() {
				wait_.lock();
				ring.begin_write(2);
				std::this_thread::sleep_for(10ms);
				ring.end_write(2);
			});
			
			wait_.unlock();
			std::this_thread::sleep_for(5ms);
			CHECK_THROWS_AS(ring.begin_write(2), sequencing_error);
			
			other.join();
		}
		
		SECTION("deadlock on read") {	
			std::mutex mut;
			mut.lock();
			std::thread writer([&](){
				// try to write 4 frames
				// --> locks, because currently only 2 writable
				mut.unlock();
				auto w_section = ring.begin_write(4);
			});
	
			// try to read 4 frames
			// --> would also lock, because currently only 3 readable
			mut.lock();
			std::this_thread::sleep_for(5ms);
			CHECK_THROWS_AS(ring.begin_read(4), sequencing_error);

			ring.skip(3);
			writer.join();
		}

		SECTION("deadlock on read") {	
			std::mutex mut;
			mut.lock();
			std::thread reader([&](){
				// try to read 4 frames
				// --> locks, because currently only 3 readable
				mut.unlock();
				auto r_section = ring.begin_read(4);
			});
	
			// try to write 4 frames
			// --> would also lock, because currently only 2 writable
			mut.lock();
			std::this_thread::sleep_for(5ms);
			CHECK_THROWS_AS(ring.begin_write(4), sequencing_error);

			ring.begin_write(1);
			ring.end_write(1);
			reader.join();
		}
	}


	SECTION("mark end") {
		auto w_section = ring.begin_write(2);
		w_section[0] = make_frame(shape, 0);
		w_section[1] = make_frame(shape, 1);
		ring.end_write(2);
		
		REQUIRE(ring.shared_readable_duration() == 2);
		REQUIRE_FALSE(ring.reader_eof());
		REQUIRE(ring.shared_writable_duration() == 3);
		REQUIRE(ring.writable_duration() == 3);
		REQUIRE_FALSE(ring.writer_eof());
		
		SECTION("writer end, read, reaches eof test") {
			// mark end at position 4 (frame 3 = last)
			auto w_section = ring.begin_write(2);
			w_section[0] = make_frame(shape, 3);
			ring.end_write(1, true);

			REQUIRE(ring.shared_readable_duration() == 3);
			REQUIRE_FALSE(ring.reader_eof());
			REQUIRE(ring.shared_writable_duration() == 0);
			REQUIRE(ring.writable_duration() == 2);
			REQUIRE(ring.writer_eof());
	
			// read less than maximum
			auto r_section = ring.begin_read(1);
			REQUIRE_FALSE(ring.read_reaches_eof());
			REQUIRE(r_section.shape()[0] == 1);
			ring.end_read(1);
			
			REQUIRE(ring.shared_readable_duration() == 2);
			REQUIRE_FALSE(ring.reader_eof());
			REQUIRE(ring.shared_writable_duration() == 0);
			REQUIRE(ring.writable_duration() == 3);
			REQUIRE(ring.writer_eof());
			
			SECTION("exact") {
				// read maximum
				r_section.reset(ring.begin_read(2));
				REQUIRE(ring.read_reaches_eof());
				ring.end_read(2);
				
				REQUIRE(ring.shared_readable_duration() == 0);
				REQUIRE(ring.reader_eof());
			}
			
			SECTION("more") {
				// read beyond maximum
				r_section.reset(ring.begin_read(3));
				REQUIRE(ring.read_reaches_eof());
				REQUIRE(r_section.shape()[0] == 2);
				ring.end_read(2);
				
				REQUIRE(ring.shared_readable_duration() == 0);
				REQUIRE(ring.reader_eof());
			}
		}
		
		SECTION("writer end, skip") {
			// mark end at position 4 (frame 3 = last)
			auto w_section = ring.begin_write(2);
			w_section[0] = make_frame(shape, 3);
			ring.end_write(1, true);
			
			// cannot write more after eof
			REQUIRE_THROWS_AS(ring.begin_write(1), sequencing_error);
			
			// skip: will not wait for any more
			ring.skip(10);
			
			REQUIRE(ring.shared_readable_duration() == 0);
			REQUIRE(ring.reader_eof());
			REQUIRE(ring.shared_writable_duration() == 0);
			REQUIRE(ring.writable_duration() == 5);
			REQUIRE(ring.writer_eof());
		}
	}
}