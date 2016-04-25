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
private:
	std::uintptr_t handle_;

	static event* wait_any_(event** begin, event** end);

public:
	event();
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
