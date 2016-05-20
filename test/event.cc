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
#include "../src/os/event.h"
#include "support/thread.h"
#include <mutex>

using namespace mf;
using namespace mf::test;

TEST_CASE("event", "[event]") {
	event ev1;
	event ev2;
	sticky_event ev3;
	
	SECTION("notify, wait") {
		ev1.notify();
		ev1.wait();
		
		ev2.notify();
		ev2.wait();
		
		ev1.notify();
		ev1.wait();
	}
	
	SECTION("sticky notify, wait") {
		ev3.notify();
		
		ev3.wait();
		ev3.wait();
		ev3.wait();
		
		ev3.reset();
		ev3.notify();
		
		ev3.wait();
	}
	
	SECTION("wait_any") {
		ev1.notify();
		event& ev_1 = event::wait_any(ev1, ev2);
		REQUIRE(ev_1 == ev1);
		
		ev2.notify();
		event& ev_2 = event::wait_any(ev1, ev2);
		REQUIRE(ev_2 == ev2);

		ev1.notify();
		ev2.notify();
		event& ev = event::wait_any(ev1, ev2);
		REQUIRE( (ev == ev1 || ev == ev2) );
		
		if(ev == ev1) {
			event& ev = event::wait_any(ev1, ev2);
			REQUIRE(ev == ev2);
		} else {
			event& ev = event::wait_any(ev1, ev2);
			REQUIRE(ev == ev1);		
		}
	}
	
	SECTION("wait_any") {
		ev1.notify();
		event& ev_1 = event::wait_any(ev1, ev3);
		REQUIRE(ev_1 == ev1);
		
		ev3.notify();
		event& ev_2 = event::wait_any(ev1, ev3);
		REQUIRE(ev_2 == ev3);	

		event& ev_3 = event::wait_any(ev1, ev3);
		REQUIRE(ev_3 == ev3);	

		ev1.notify();
		ev3.reset();
		event& ev_4 = event::wait_any(ev1, ev3);
		REQUIRE(ev_4 == ev1);
	}
	
	SECTION("thread") {
		std::mutex mut;
		mut.lock();
		MF_TEST_THREAD() {
			mut.lock();
			ev1.notify();
		};
		
		mut.unlock();
		ev1.wait();
	}
}
