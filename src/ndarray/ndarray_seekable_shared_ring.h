#ifndef MF_NDARRAY_SEEKABLE_SHARED_RING_H_
#define MF_NDARRAY_SEEKABLE_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>
#include "ndarray_timed_ring.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_seekable_shared_ring {
public:
	using ring_type = ndarray_timed_ring<Dim, T>;
	using section_view_type = typename ring_type::section_view_type;
	
private:
	enum thread_state { idle, waiting, accessing };

	tume_unit end_time_;

	ring_type ring_; ///< Underlying, non thread-safe timed ring buffer.

	mutable std::mutex mutex_; ///< Protects read/write positions from concurrent access, and waiting for frames.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.

	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
	// can also be computed as end_time_ + 1 - ring_.readable_duration().
	// however this would require mutex lock, because both terms are altered by writer in end_write()
		
	void skip_available_(time_unit duration);
	
public:
	ndarray_seekable_shared_ring(const ndsize<Dim>& frames_shape, std::size_t duration, time_unit end_time);
		
	void initialize();

	time_unit total_duration() const noexcept { return ring_.shape().front(); }

	section_view_type begin_write(time_unit duration);
	void end_write(time_unit written_duration);

	section_view_type begin_read_span(time_span);
	section_view_type begin_read_span(time_span, bool& reaches_eof);
	section_view_type begin_read(time_unit read_duration);
	section_view_type begin_read(time_unit read_duration, bool& reaches_eof);
	void end_read(time_unit read_duration);	
	
	void skip(time_unit skip_duration);
	void skip_span(time_span);	
	
	void seek(time_unit target_time);
	
	time_unit current_time() const noexcept;
	time_unit write_start_time() const noexcept;
	time_unit read_start_time() const noexcept;

	time_unit writable_duration() const;
	time_span writable_time_span() const;
	time_unit readable_duration() const;	
	time_span readable_time_span() const;
		
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_seekable_shared_ring.tcc"

#endif
