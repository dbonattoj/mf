#include <catch.hpp>
#include "../src/utility/event.h"
#include "support/thread.h"
#include <mutex>

using namespace mf;
using namespace mf::test;

TEST_CASE("event", "[event]") {
	event ev1;
	event ev2;
	
	SECTION("notify, wait") {
		ev1.notify();
		ev1.wait();
		
		ev2.notify();
		ev2.wait();
		
		ev1.notify();
		ev1.wait();
	}
	
	SECTION("wait_any") {
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
