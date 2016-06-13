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
#include <atomic>
#include <chrono>
#include <algorithm>
#include <initializer_list>

namespace mf {
	
using event_id = std::uintptr_t;

/// Synchronization primitive representing event that a thread can wait for.
/** Call to receive() blocks until event has been _sent_ from another thread via send(). That is, until the event
 ** is _received_. If multiple threads wait on the same event, one (undefined which) receives it.
 ** Event is received once after one or multiple calls to send(). Next call to receive() blocks again, multiple send()
 ** calls are not accumulated. */
class event {
protected:
	std::uintptr_t handle_;

public:
	event();
	event(const event&) = delete;
	event(event&&);
	virtual ~event();
	
	event_id id() const noexcept { return handle_; }
	std::uintptr_t handle() const noexcept { return handle_; }
	
	event& operator=(const event&) = delete;
	event& operator=(event&&);
	
	friend bool operator==(const event& a, const event& b);
	friend bool operator!=(const event& a, const event& b) { return !(a == b); }
	
	virtual void send();
	virtual void receive();
	virtual bool receive(std::chrono::milliseconds timeout);
};


/// Event which is repeatedly received after having been sent once.
/** Altered version of \ref event. After send(), all subsequent receive() calls receive the event without
 ** waiting, until the event is reset using reset(). */
class sticky_event : public event {
private:
	std::atomic<bool> sent_ {false};
	
public:
	sticky_event() = default;
	~sticky_event();

	void send() override;
	void receive() override;
	bool receive(std::chrono::milliseconds timeout) override;
	
	void reset();
	bool was_sent() { return sent_; }
};


class event_set {
private:
	std::set<event*> events_;
	std::chrono::milliseconds timeout_ {-1};

public:
	event_set() = default;
	event_set(event& ev) { add_event(ev); }
	event_set(std::initializer_list<std::reference_wrapper<event>> evs) {
		for(event& ev : evs) add_event(ev);
	}

	static constexpr event_id timeout_id = 0;
	
	void add_event(event& ev) { events_.insert(&ev); }
	void delete_event(event& ev) { events_.erase(&ev); }
	bool has_event(event& ev) const { return (events_.find(&ev) != events_.end()); }
	
	bool has_event(event_id id) const {
		return std::any_of(events_.begin(), events_.end(), [id](event* ev) { return ev->id() == id; });
	}
	
	void set_timeout(const std::chrono::milliseconds& timeout) { timeout_ = timeout; }
	void set_no_timeout() { timeout_ = std::chrono::milliseconds(-1); }
	
	event_id receive_any();
};

}

#endif
