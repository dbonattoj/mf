#ifndef MF_NDARRAY_SEEKABLE_SHARED_RING_H_
#define MF_NDARRAY_SEEKABLE_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>
#include "ndarray_shared_ring_base.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_seekable_shared_ring : public ndarray_shared_ring_base<Dim, T> {
	using base = ndarray_shared_ring_base<Dim, T>;
	
private:
	using thread_state = time_unit;
	enum : thread_state {
		idle = 0,
		accessing = -1
	};
	// positive number = number of frames waiting for

	using typename base::ring_type;
	using typename base::section_view_type;

	ring_type ring_; ///< Underlying, non thread-safe timed ring buffer.

	time_unit end_time_;

	mutable std::mutex mutex_; ///< Protects read/write positions from concurrent access, and waiting for frames.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.


	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
	// can also be computed as end_time_ + 1 - ring_.readable_duration().
	// however this would require mutex lock, because both terms are altered by writer in end_write()
	
public:
	ndarray_seekable_shared_ring(const ndsize<Dim>& frames_shape, std::size_t capacity, time_unit end_time);
		
	void initialize();

	time_unit capacity() const override { return ring_.shape().front(); }

	section_view_type begin_write(time_unit duration) override;
	void end_write(time_unit written_duration) override;

	section_view_type begin_read_span(time_span) override;
	section_view_type begin_read(time_unit read_duration) override;
	void end_read(time_unit read_duration) override;	
	
	void skip(time_unit skip_duration) override;
	
	void seek(time_unit target_time) override;
	
	time_unit current_time() const override;
	
	time_unit write_start_time() const override;
	time_unit read_start_time() const override;
	time_span writable_time_span() const override;
	time_span readable_time_span() const override;
	
	bool end_time_is_defined() const override { return true; }
	time_unit end_time() const override { return end_time_; }
		
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_seekable_shared_ring.tcc"

#endif
