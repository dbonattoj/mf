#include <catch.hpp>
#include "../src/utility/event.h"
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
