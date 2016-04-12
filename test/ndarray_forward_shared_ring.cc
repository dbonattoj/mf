#include <catch.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include "../src/ndarray/ndarray_forward_shared_ring.h"
#include "support/ndarray.h"

using namespace mf;
using namespace mf::test;
using namespace std::literals;

TEST_CASE("ndarray_forward_shared_ring", "[ndarray_shared_ring][parallel]") {
	ndsize<2> shape{320, 240};
	std::size_t duration = 5;
	bool reaches_eof;
	
	ndarray_forward_shared_ring<2, int> ring(shape, duration);
	REQUIRE(ring.read_start_time() == 0);
	REQUIRE(ring.write_start_time() == 0);
	
	REQUIRE(ring.capacity() == 5);

	SECTION("background writer") {
		// secondary thread: keeps writing frames into buffer
		std::atomic_bool running(true);
		std::thread writer([&]() {
			int t = 0;
			while(running) {
				std::this_thread::sleep_for(5ms);
				auto w_section = ring.begin_write(1);
				REQUIRE(w_section.duration() == 1);
				REQUIRE(t == w_section.start_time());
				REQUIRE(w_section.time_at(0) == t);
				w_section[0] = make_frame(shape, t++);
				ring.end_write(1);
			}
		});
				
		// read 3 frames (0, 1, 2)
		auto r_section = ring.begin_read(3);
		REQUIRE(r_section.duration() == 3);
		REQUIRE(r_section.start_time() == 0);
		REQUIRE(ring.write_start_time() >= 3);
		REQUIRE(compare_frames(shape, r_section, {0, 1, 2}));
		ring.end_read(3);
		REQUIRE(ring.current_time() >= 2);
		REQUIRE(ring.read_start_time() == 3);
		
		// skip 2 frames (3, 4)
		ring.skip(2);
		REQUIRE(ring.current_time() >= 4);
		REQUIRE(ring.write_start_time() >= 5);
		REQUIRE(ring.read_start_time() == 5);
	
		// try to read frames span (4, 5, 6) (already passed)
		REQUIRE_THROWS_AS(ring.begin_read_span(time_span(4, 7)), sequencing_error);
		REQUIRE(ring.current_time() >= 4);
		REQUIRE(ring.write_start_time() >= 5);
		REQUIRE(ring.read_start_time() == 5);
		
		// read frames (5, 6)
		r_section.reset(ring.begin_read_span(time_span(5, 7)));
		REQUIRE(r_section.duration() == 2);
		REQUIRE(r_section.start_time() == 5);
		REQUIRE(compare_frames(shape, r_section, {5, 6}));
		ring.end_read(2);
		REQUIRE(ring.current_time() >= 6);
		
		// read frames (20, 21)
		// skip previous
		r_section.reset(ring.begin_read_span(time_span(20, 22)));
		REQUIRE(compare_frames(shape, r_section, {20, 21}));
		ring.end_read(2);
		REQUIRE(ring.current_time() >= 21);
		
		// skip 100 frames
		// more than ring capacity
		ring.skip(100);
		REQUIRE(ring.current_time() >= 121);
		
		// end secondary thread
		running = false;
		writer.join();
	}
	
	
	SECTION("wait for writer") {
		// reader thread: waits until frames become available
		std::atomic<bool> started{false};
		std::thread reader([&]() {
			auto r_section = ring.begin_read(3);
			started = true;
			REQUIRE(compare_frames(shape, r_section, {0, 1, 2}));
			ring.end_read(3);
		});

		// make sure reader has not started
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write 1 frame -> not enough
		auto w_section = ring.begin_write(1);
		w_section[0] = make_frame(shape, 0);
		ring.end_write(1);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write 2 frames -> now reader starts
		w_section.reset(ring.begin_write(2));
		w_section[0] = make_frame(shape, 1);
		w_section[1] = make_frame(shape, 2);
		ring.end_write(2);
		std::this_thread::sleep_for(10ms);
		REQUIRE(started);
		
		reader.join();	
	}
	
	
	SECTION("wait for writer (span)") {
		ndarray_forward_shared_ring<2, int> ring(shape, 20); // use longer ring
		
		// reader thread: waits until frames [10, 14[
		std::atomic<bool> started{false};
		std::thread reader([&]() {
			auto r_section = ring.begin_read_span(time_span(10, 14));
			started = true;
			REQUIRE(compare_frames(shape, r_section, {10, 11, 12, 13}));
			ring.end_read(4);
		});

		// make sure reader has not started
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write ]0, 8], not enough
		auto w_section = ring.begin_write(9);
		for(int i = 0; i < 8; ++i) w_section[i] = make_frame(shape, i);
		ring.end_write(8);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write ]8, 12], still not enough
		w_section.reset(ring.begin_write(5));
		for(int i = 0; i < 4; ++i) w_section[i] = make_frame(shape, i + 8);
		ring.end_write(4);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write ]12, 13], still not enough
		w_section.reset(ring.begin_write(2));
		for(int i = 0; i < 1; ++i) w_section[i] = make_frame(shape, i + 12);
		ring.end_write(1);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write ]13, 15], now reading
		w_section.reset(ring.begin_write(3));
		for(int i = 0; i < 2; ++i) w_section[i] = make_frame(shape, i + 13);
		ring.end_write(2);
		std::this_thread::sleep_for(10ms);
		REQUIRE(started);

		reader.join();	
	}
	
	
	SECTION("wait for reader") {
		// fill up 4/5 of buffer
		auto w_section = ring.begin_write(4);
		for(int i = 0; i < 4; ++i) w_section[i] = make_frame(shape, i);
		ring.end_write(4);
		REQUIRE(ring.writable_duration() == 1);
		
		// writer thread: waits until 4 frames become available
		std::atomic<bool> started{false};
		std::thread writer([&]() {
			auto w_section = ring.begin_write(4);
			started = true;
			for(int i = 0; i < 4; ++i) w_section[i] = make_frame(shape, i + 4);
			ring.end_write(4);
		});

		// make sure writer has not started
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// read 1 frame -> not enough
		auto r_section = ring.begin_read(1);
		REQUIRE(compare_frames(shape, r_section, {0}));
		ring.end_read(1);
		REQUIRE(ring.writable_duration() == 2);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);

		// skip 1 frame -> not enough
		ring.skip(1);
		REQUIRE(ring.writable_duration() == 3);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// read 2 frames -> now writer starts
		r_section.reset(ring.begin_read(2));
		REQUIRE(compare_frames(shape, r_section, {2, 3}));
		ring.end_read(2);
		std::this_thread::sleep_for(10ms);
		REQUIRE(started);
		
		// now (wait and) read 4 new frames
		r_section.reset(ring.begin_read(4));
		REQUIRE(compare_frames(shape, r_section, {4, 5, 6, 7}));
		ring.end_read(4);
		
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

		SECTION("deadlock on write") {	
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
		
		REQUIRE(ring.readable_duration() == 2);
		REQUIRE(ring.writable_duration() == 3);

		REQUIRE_FALSE(ring.end_time_is_defined());
		REQUIRE(ring.end_time() == -1);
		REQUIRE_FALSE(ring.writer_reached_end());
		REQUIRE_FALSE(ring.reader_reached_end());
		
		SECTION("writer end, read, reaches eof test") {
			// mark end at position 3 (frame 2 = last)
			auto w_section = ring.begin_write(2);
			w_section[0] = make_frame(shape, 2);
			ring.end_write(1, true);

			REQUIRE(ring.readable_duration() == 3);
			REQUIRE(ring.writable_duration() == 0);
			REQUIRE(ring.writer_reached_end());
			REQUIRE(ring.end_time_is_defined());
			REQUIRE(ring.end_time() == 3);
			REQUIRE_FALSE(ring.reader_reached_end());
	
			// read less than maximum
			reaches_eof = true; // ...should be set to false
			auto r_section = ring.begin_read(1, reaches_eof);
			REQUIRE_FALSE(reaches_eof);
			REQUIRE(r_section.shape()[0] == 1);
			ring.end_read(1);
			
			REQUIRE(ring.readable_duration() == 2);
			REQUIRE(ring.writable_duration() == 0);
			REQUIRE(ring.writer_reached_end());
			REQUIRE(ring.end_time_is_defined());
			REQUIRE(ring.end_time() == 3);
			REQUIRE_FALSE(ring.reader_reached_end());
		
			SECTION("exact") {
				// read maximum
				reaches_eof = false;
				r_section.reset(ring.begin_read(2, reaches_eof));
				REQUIRE(reaches_eof);
				ring.end_read(2);
				
				REQUIRE(ring.readable_duration() == 0);
				REQUIRE(ring.reader_reached_end());
			}
			
			SECTION("more") {
				// read beyond maximum
				reaches_eof = false;
				r_section.reset(ring.begin_read(3, reaches_eof));
				REQUIRE(reaches_eof);
				REQUIRE(r_section.shape()[0] == 2);
				ring.end_read(2);
				
				REQUIRE(ring.readable_duration() == 0);
				REQUIRE(ring.reader_reached_end());
			}
		}
		
		
		SECTION("writer end, skip") {
			// mark end at position 3 (frame 2 = last)
			auto w_section = ring.begin_write(2);
			w_section[0] = make_frame(shape, 2);
			ring.end_write(1, true);
			REQUIRE(ring.end_time_is_defined());
			REQUIRE(ring.end_time() == 3);
			
			// cannot write more after eof
			REQUIRE_THROWS_AS(ring.begin_write(1), sequencing_error);
			
			// skip: will not wait for any more
			ring.skip(10);
			
			REQUIRE(ring.readable_duration() == 0);
			REQUIRE(ring.writable_duration() == 0);
			REQUIRE(ring.writer_reached_end());
			REQUIRE(ring.reader_reached_end());
		}
	}
}
