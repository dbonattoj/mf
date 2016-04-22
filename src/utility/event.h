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
/** Internal counter is initialized to zero. Call to notify() increases internal counter by specified amount.
 ** When counter is non-zero, call to wait() resets counter and returns its old value. When the counter is zero
 ** wait() blocks until it becomes non-zero and then proceeds as before.
 ** Only one thread can wait() at the same time. It is possible to wait on multiple events using wait_any(). */
class event {
public:
	using counter_type = std::size_t;
	using handle_type = std::uintptr_t;

private:
	handle_type handle_;

public:
	handle_type handle() noexcept { return handle_; }

	event();
	event(const event&) = delete;
	event(event&&);
	~event();
	
	event& operator=(const event&) = delete;
	event& operator=(event&&);
	
	friend bool operator==(const event& a, const event& b);
	friend bool operator!=(const event& a, const event& b) { return !(a == b); }
	
	void notify(counter_type = 1);
	counter_type wait();
};


struct event_wait_result {
	event& received_event;
	event::counter_type counter;
};


namespace detail {
	event_wait_result wait_any(event** begin, event** end);
}



template<typename It>
auto wait_any(It begin_it, It end_it)
-> std::enable_if_t<
	std::is_same<
		typename std::iterator_traits<It>::value_type,
		event*
	>::value,
	event_wait_result
>{
	event** begin = std::addressof(*begin_it);
	event** end = std::addressof(*end_it);
	MF_ASSERT_MSG( (end - begin) == std::distance(begin_it, end_it), "iterators must be contiguous");
	return detail::wait_any(begin, end);
}


template<typename... Events>
auto wait_any(Events&&... events)
-> std::common_type_t<
	std::enable_if_t<
		std::is_convertible<Events, event&>::value,
		event_wait_result
	>... 
>{
	std::array<event*, sizeof...(Events)> evs { &events... };
	return wait_any(evs.begin(), evs.end());
}


}

#endif
