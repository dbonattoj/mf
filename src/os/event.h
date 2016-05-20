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

#ifndef MF_UTILITY_EVENT_H_
#define MF_UTILITY_EVENT_H_

#include "../common.h"
#include <cstdint>
#include <utility>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <array>

namespace mf {

/// Synchronization primitive representing event that a thread can wait for.
/** Call to wait() blocks until event has been _notified_ from another thread via notify(). That is, until the event
 ** is _received_. wait_any() waits for multiple events, until any one has been notified. If multiple threads wait on
 ** the same event, one (undefined which) receives it.
 ** Event is received once after one or multiple calls to notify(). Next call to wait() blocks again, multiple notify()
 ** calls are not accumulated. */
class event {
public:
	std::uintptr_t handle_; ///< OS-specific handle.

	static event* wait_any_(event** begin, event** end);

public:
	event();
	event(const event&) = delete;
	event(event&&);
	virtual ~event();
	
	event& operator=(const event&) = delete;
	event& operator=(event&&);
	
	friend bool operator==(const event& a, const event& b);
	friend bool operator!=(const event& a, const event& b) { return !(a == b); }
	
	virtual void notify();
	virtual void wait();
	
	template<typename It>
	static event& wait_any_list(It begin_it, It end_it) {
		static_assert(
			std::is_same<typename std::iterator_traits<It>::value_type, event*>::value,
			"arguments must be contiguous iterators with value_type event*"
		);
		event** begin = std::addressof(*begin_it);
		event** end = std::addressof(*end_it);
		MF_ASSERT_MSG( (end - begin) == std::distance(begin_it, end_it), "iterators must be contiguous");
		return *wait_any_(begin, end);
	}

	template<typename... Events>
	static event& wait_any(Events&&... events) {
		std::array<event*, sizeof...(Events)> evs { &events... };
		return wait_any_list(evs.begin(), evs.end());
	}
};


/// Event which is repeatedly received after having been notified once.
/** Altered version of \ref event. After notified(), all subsequent wait() or wait_any() calls receive the event without
 ** waiting, until the event is reset using reset(). */
class sticky_event : public event {
private:
	bool notified_ = false;
	
public:
	sticky_event() = default;

	void notify() override {
		notified_ = true;
		event::notify();
	}
	
	void wait() override {
		event::wait();
		event::notify();
	}
	
	void reset() {
		if(notified_) event::wait();
		notified_ = false;
	}
};

}

#endif
