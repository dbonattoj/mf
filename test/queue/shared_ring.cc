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
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <catch.hpp>
#include <atomic>
#include <thread>
#include <chrono>
#include <mf/queue/shared_ring.h>
#include "../support/ndarray_opaque.h"
#include "../support/thread.h"

using namespace mf;
using namespace mf::test;
using namespace std::literals;


TEST_CASE("shared_ring", "[queue][shared_ring]") {
	auto frm = opaque_frame_format();
	std::size_t duration = 5;
	bool reaches_eof;
	
	shared_ring rng(frm, duration, 200);
	REQUIRE(rng.read_start_time() == 0);
	REQUIRE(rng.write_start_time() == 0);
	
	REQUIRE(rng.end_time() == 200);	
	REQUIRE(rng.capacity() == 5);
	
	SECTION("break reader") {
		SECTION("reject write") {
			std::atomic<bool> done(false);
			MF_TEST_THREAD() {
				while(! done) {
					// don't write 1 frame
					auto w_section = rng.begin_write(1);
					rng.end_write(0);
				
					// break reader
					rng.break_reader();
				}
			};
			
			// read 3 frames, wait for them to become available...
			auto r_section = rng.begin_read(3);
			REQUIRE(r_section.is_null());
			
			done = true;
		}
		
		SECTION("not reading") {
			std::atomic<bool> broke(false), cont(false);
			MF_TEST_THREAD() {
				rng.break_reader();
				broke = true;
				
				while(! cont);
				
				auto w_section = rng.begin_write(3);
				rng.end_write(3);
			};
			
			while(! broke);
			cont = true;
			
			auto r_section = rng.begin_read(3);
			REQUIRE(r_section.duration() == 3);
			rng.end_read(3);
		}
	}
	

	
	SECTION("break writer") {
		// fill buffer
		rng.begin_write(duration);
		rng.end_write(duration);

		SECTION("reject read") {
			std::atomic<bool> done(false);
			MF_TEST_THREAD() {
				while(! done) {
					// don't read 1 frame
					auto r_section = rng.begin_read(1);
					rng.end_read(0);
				
					// break writer
					rng.break_writer();
				}
			};
			
			// write 3 frames, wait for them to become available...
			auto w_section = rng.begin_write(3);
			REQUIRE(w_section.is_null());
			
			done = true;
		}
		
		SECTION("not writing") {
			std::atomic<bool> broke(false), cont(false);
			MF_TEST_THREAD() {
				rng.break_writer();
				broke = true;
				
				while(! cont);
				
				auto w_section = rng.begin_read(3);
				rng.end_read(3);
			};
			
			while(! broke);
			cont = true;
			
			auto w_section = rng.begin_write(3);
			REQUIRE(w_section.duration() == 3);
			rng.end_write(3);
		}
	}



	SECTION("background writer") {
		// secondary thread: keeps writing frames into buffer
		std::atomic<bool> discontinuity(false);
		MF_TEST_THREAD() {
			int t = 0;
			for(;;) {
				auto w_section = rng.begin_write(1);
				if(w_section.duration() == 0) break; // end when eof reached
				MF_TEST_THREAD_REQUIRE(w_section.duration() == 1);
				w_section[0] = make_opaque_frame(w_section.start_time());
				rng.end_write(1);
				
				if(w_section.start_time() != t++) discontinuity = true;
			}
		};

		// read 3 frames (0, 1, 2)
		auto r_section = rng.begin_read(3);
		REQUIRE(r_section.duration() == 3);
		REQUIRE(r_section.start_time() == 0);
		REQUIRE(rng.write_start_time() >= 3);
		REQUIRE(compare_opaque_frames(r_section, {0, 1, 2}));
		rng.end_read(3);
		REQUIRE(rng.current_time() >= 2);
		REQUIRE(rng.read_start_time() == 3);

		// read frames span (4, 5, 6) (skip 2)
		r_section.reset(rng.begin_read_span(time_span(4, 7)));
		REQUIRE(r_section.duration() == 3);
		REQUIRE(r_section.start_time() == 4);
		REQUIRE(compare_opaque_frames(r_section, {4, 5, 6}));
		rng.end_read(3);
		REQUIRE(rng.current_time() >= 6);
		REQUIRE(rng.read_start_time() == 7);

		// skip 2 frames (7, 8)
		rng.skip(2);
		REQUIRE(rng.current_time() >= 8);
		REQUIRE(rng.write_start_time() >= 9);
		REQUIRE(rng.read_start_time() == 9);

		// skip 0 frames
		rng.skip(0);
		REQUIRE(rng.current_time() >= 8);
		REQUIRE(rng.write_start_time() >= 9);
		REQUIRE(rng.read_start_time() == 9);
								
		// read 3 frames (9, 10, 11), request 5
		r_section.reset(rng.begin_read(5));
		REQUIRE(r_section.duration() == 5);
		REQUIRE(r_section.start_time() == 9);
		REQUIRE(rng.write_start_time() >= 12);
		REQUIRE(compare_opaque_frames(r_section, {9, 10, 11, 12, 13}));
		rng.end_read(3);
		REQUIRE(rng.current_time() >= 13);
		REQUIRE(rng.write_start_time() >= 14);
		REQUIRE(rng.read_start_time() == 12);
		
		// read 0 frames, request 3
		r_section.reset(rng.begin_read(3));
		rng.end_read(0);
		REQUIRE(rng.current_time() >= 13);
		REQUIRE(rng.write_start_time() >= 14);
		REQUIRE(rng.read_start_time() == 12);		

		// read frames (20, 21)
		// skip previous (--> seek, necessarily because inbetween frames don't fit in buffer capacity)
		discontinuity = false;
		r_section.reset(rng.begin_read_span(time_span(20, 22)));

		REQUIRE(compare_opaque_frames(r_section, {20, 21}));
		rng.end_read(2);
		REQUIRE(rng.current_time() >= 21);
		REQUIRE(rng.write_start_time() >= 22);
		REQUIRE(rng.read_start_time() == 22);
		REQUIRE(discontinuity);

		// seek to future
		discontinuity = false;
		r_section.reset(rng.begin_read_span(time_span(150, 153)));
		REQUIRE(r_section.duration() == 3);
		REQUIRE(r_section.start_time() == 150);
		REQUIRE(compare_opaque_frames(r_section, {150, 151, 152}));
		rng.end_read(3);
		REQUIRE(rng.current_time() >= 152);
		REQUIRE(rng.write_start_time() >= 153);
		REQUIRE(rng.read_start_time() == 153);
		REQUIRE(discontinuity);
		
		// seek to past
		discontinuity = false;
		rng.seek(50);
		r_section.reset(rng.begin_read_span(time_span(50, 53)));
		REQUIRE(r_section.duration() == 3);
		REQUIRE(r_section.start_time() == 50);
		REQUIRE(rng.write_start_time() >= 3);
		REQUIRE(compare_opaque_frames(r_section, {50, 51, 52}));
		rng.end_read(3);
		REQUIRE(rng.current_time() >= 52);
		REQUIRE(rng.write_start_time() >= 53);
		REQUIRE(rng.read_start_time() == 53);
		REQUIRE(discontinuity);

		// attempt to seek beyond end
		REQUIRE_THROWS(rng.seek(201));
		
		// seek near end
		rng.seek(198);
		r_section.reset(rng.begin_read(5));
		REQUIRE(r_section.duration() == 2); // must be truncated
		REQUIRE(r_section.start_time() == 198);
		REQUIRE(compare_opaque_frames(r_section, {198, 199}));
		rng.end_read(2);
		// will end writer, because writer arrived at end and begin_write().duration() == 0
	}


	SECTION("seek while writing") {
		// fill up 4/5 of buffer
		auto w_section = rng.begin_write(4);
		for(int i = 0; i < 4; ++i) w_section[i] = make_opaque_frame(i);
		rng.end_write(4);
		REQUIRE(rng.writable_duration() == 1);
		
		// writer thread: waits until 4 frames become available
		std::atomic<bool> started{false};
		MF_TEST_THREAD() {
			auto w_section = rng.begin_write(4);
			started = true;
			for(int i = 0; i < w_section.duration(); ++i) w_section[i] = make_opaque_frame(w_section.time_at(i));
			rng.end_write(w_section.duration());
		};

		// seek while writer is waiting
		rng.seek(100);
	}
	

	SECTION("wait for writer, seek") {
		// reader thread: waits until frames become available
		std::atomic<bool> started{false};
		MF_TEST_THREAD() {
			auto r_section = rng.begin_read_span(time_span(100, 103));
			started = true;
			REQUIRE(compare_opaque_frames(r_section, {100, 101, 102}));
			rng.end_read(3);
		};

		// make sure reader has not started
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write 1 frame -> not enough
		auto w_section = rng.begin_write(1);
		w_section[0] = make_opaque_frame(w_section.start_time());
		rng.end_write(1);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// write 2 frames -> now reader starts
		w_section.reset(rng.begin_write(2));
		w_section[0] = make_opaque_frame(w_section.time_at(0));
		w_section[1] = make_opaque_frame(w_section.time_at(1));
		rng.end_write(2);
		std::this_thread::sleep_for(10ms);
		REQUIRE(started);
	}


	SECTION("wait for reader") {
		// fill up 4/5 of buffer
		auto w_section = rng.begin_write(4);
		for(int i = 0; i < 4; ++i) w_section[i] = make_opaque_frame(i);
		rng.end_write(4);
		REQUIRE(rng.writable_duration() == 1);
		
		// writer thread: waits until 4 frames become available
		std::atomic<bool> started{false};
		MF_TEST_THREAD() {
			auto w_section = rng.begin_write(4);
			started = true;
			for(int i = 0; i < 4; ++i) w_section[i] = make_opaque_frame(i + 4);
			rng.end_write(4);
		};

		// make sure writer has not started
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// read 1 frame -> not enough
		auto r_section = rng.begin_read(1);
		REQUIRE(compare_opaque_frames(r_section, {0}));
		rng.end_read(1);
		REQUIRE(rng.writable_duration() == 2);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);

		// skip 1 frame -> not enough
		rng.skip(1);
		REQUIRE(rng.writable_duration() == 3);
		std::this_thread::sleep_for(10ms);
		REQUIRE_FALSE(started);
		
		// read 2 frames -> now writer starts
		r_section.reset(rng.begin_read(2));
		REQUIRE(compare_opaque_frames(r_section, {2, 3}));
		rng.end_read(2);
		std::this_thread::sleep_for(10ms);
		REQUIRE(started);
		
		// now (wait and) read 4 new frames
		r_section.reset(rng.begin_read(4));
		REQUIRE(compare_opaque_frames(r_section, {4, 5, 6, 7}));
		rng.end_read(4);
	}



	SECTION("invalid use") {
		auto w_section = rng.begin_write(3);
		w_section[0] = make_opaque_frame(w_section.time_at(0));
		w_section[1] = make_opaque_frame(w_section.time_at(1));
		w_section[2] = make_opaque_frame(w_section.time_at(2));
		rng.end_write(3);
		
		REQUIRE(rng.readable_duration() == 3);
		REQUIRE(rng.writable_duration() == 2);

		std::atomic<bool> ready{false};

		SECTION("concurrent reading") {
			MF_TEST_THREAD() {
				rng.begin_read(2);
				ready = true;
				while(ready);
			};
			
			while(! ready);
			// it should be reading now...
			REQUIRE_THROWS_AS(rng.begin_read(2), sequencing_error);
			ready = false;
		}
		
		SECTION("concurrent writing") {
			MF_TEST_THREAD() {
				rng.begin_write(2);
				ready = true;
				while(ready);
			};
			
			while(! ready);
			REQUIRE_THROWS_AS(rng.begin_write(2), sequencing_error);
			ready = false;
		}
	
		SECTION("deadlock on read") {	
			std::mutex mut;
			mut.lock();
			MF_TEST_THREAD() {
				// try to write 4 frames
				// --> locks, because currently only 2 writable
				mut.unlock();
				auto w_section = rng.begin_write(4);
			};
	
			// try to read 4 frames
			// --> would also lock, because currently only 3 readable
			mut.lock();
			std::this_thread::sleep_for(5ms);
			CHECK_THROWS_AS(rng.begin_read(4), sequencing_error);

			// let writer end...
			rng.begin_read(3);
			rng.end_read(3);
		}
		
		SECTION("deadlock on write") {	
			std::mutex mut;
			mut.lock();
			MF_TEST_THREAD() {
				// try to read 4 frames
				// --> locks, because currently only 3 readable
				mut.unlock();
				auto r_section = rng.begin_read(4);
			};
	
			// try to write 4 frames
			// --> would also lock, because currently only 2 writable
			mut.lock();
			std::this_thread::sleep_for(5ms);
			CHECK_THROWS_AS(rng.begin_write(4), sequencing_error);

			rng.begin_write(1);
			rng.end_write(1);
		}
	}
}
