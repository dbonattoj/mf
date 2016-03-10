#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <string>
#include "ndarray_timed_ring.h"

namespace mf {

/// Ndarray timed ring for concurrent read/write access.
/** For use with two threads: one which reads and one which writes. Does not support having multiple parallel reads
 ** or multiple parallel writes. Enhancements:
 ** - Mutex lock on buffer state (read and write positions)
 ** - Semantics of begin_write(), begin_read() and skip() changed to wait until frames become available 
 ** Base class functions which depend on the read/write positions (e.g. readable_duration(), writable_time_span(),
 ** etc) are not thread-safe. Class is \c Lockable for this. **/
template<std::size_t Dim, typename T>
class ndarray_shared_ring : public ndarray_timed_ring<Dim, T> {
	using base = ndarray_timed_ring<Dim, T>;

private:
	mutable std::mutex positions_mutex_; ///< Mutex which gets locked while advancing pointers.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.

	void skip_available_(std::size_t duration);
	
public:
	using typename base::section_view_type;

	ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t duration, time_unit time_offset = 0) :
		base(frames_shape, duration, time_offset) { }
	
	section_view_type begin_write(std::size_t duration) override;
	void end_write(std::size_t written_duration) override;

	section_view_type begin_read(std::size_t duration) override;
	void end_read(std::size_t written_duration) override;	
	
	void skip(std::size_t duration) override;
	
	void lock() { positions_mutex_.lock(); }
	bool try_lock() { return positions_mutex_.try_lock(); }
	void unlock() { positions_mutex_.unlock(); }
};

}

#include "ndarray_shared_ring.tcc"

#endif
