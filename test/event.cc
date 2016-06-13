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
	
	event_set ev12({ev1, ev2});
	event_set ev13({ev1, ev3});
	
	SECTION("send, receive") {
		ev1.send();
		ev1.receive();
		
		ev2.send();
		ev2.receive();
		
		ev1.send();
		ev1.receive();
	}
	
	SECTION("sticky send, receive") {
		ev3.send();
		
		ev3.receive();
		ev3.receive();
		ev3.receive();
		
		ev3.reset();
		ev3.send();
		
		ev3.receive();
	}
	
	SECTION("receive_any") {		
		ev1.send();
		event_id ev_1 = ev12.receive_any();
		REQUIRE(ev_1 == ev1.id());
		
		ev2.send();
		event_id ev_2 = ev12.receive_any();
		REQUIRE(ev_2 == ev2.id());

		ev1.send();
		ev2.send();
		event_id ev = ev12.receive_any();
		REQUIRE( (ev == ev1.id() || ev == ev2.id()) );
		
		if(ev == ev1.id()) {
			event_id ev = ev12.receive_any();
			REQUIRE(ev == ev2.id());
		} else {
			event_id ev = ev12.receive_any();
			REQUIRE(ev == ev1.id());		
		}
	}
	
	SECTION("receive_any, sticky") {
		ev1.send();
		event_id ev_1 = ev13.receive_any();
		REQUIRE(ev_1 == ev1.id());

		ev3.send();
		event_id ev_2 = ev13.receive_any();
		REQUIRE(ev_2 == ev3.id());	

		event_id ev_3 = ev13.receive_any();
		REQUIRE(ev_3 == ev3.id());	

		ev1.send();
		ev3.reset();
		event_id ev_4 = ev13.receive_any();
		REQUIRE(ev_4 == ev1.id());
	}
	
	SECTION("thread") {
		std::mutex mut;
		mut.lock();
		MF_TEST_THREAD() {
			mut.lock();
			ev1.send();
		};
		
		mut.unlock();
		ev1.receive();
	}
}
