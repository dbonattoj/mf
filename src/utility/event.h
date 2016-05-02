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
 ** Event can be _sticky_ or _non-sticky_ (default). For non-sticky event, event is received once after one or multiple
 ** calls to notify(). Next call to wait() blocks again, multiple notify() calls are not accumulated.
 ** For sticky event, it is received repeatedly after having been notified once. */
class event {
public:
	std::uintptr_t handle_;
	bool sticky_;

	static event* wait_any_(event** begin, event** end);

public:
	explicit event(bool sticky = false);
	event(const event&) = delete;
	event(event&&);
	~event();
	
	event& operator=(const event&) = delete;
	event& operator=(event&&);
	
	friend bool operator==(const event& a, const event& b);
	friend bool operator!=(const event& a, const event& b) { return !(a == b); }
	
	void notify();
	void wait();
	
	
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

}

#endif
