#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>
#include "ndarray_timed_ring.h"

namespace mf {

/// Ndarray timed ring for concurrent read/write access.
/** For use with two threads: one which reads and one which writes. Does not support having multiple parallel reads
 ** or multiple parallel writes. Enhancements:
 ** - Mutex lock on buffer state (read and write positions)
 ** - Semantics of begin_write(), begin_read() and skip() changed to wait until frames become available 
 ** - Writer can mark end of file. begin_read(), begin_read_span() and skip() then to not wait for more frames
 ** Does not inherit from base class because thread safety is added. Provides same interface. **/
template<std::size_t Dim, typename T>
class ndarray_shared_ring {
public:
	using ring_type = ndarray_timed_ring<Dim, T>;
	using section_view_type = typename ring_type::section_view_type;

private:
	enum thread_state { idle, waiting, accessing };

	ring_type ring_; ///< Underlying, non thread-safe timed ring buffer.

	mutable std::mutex mutex_; ///< Protects read/write positions from concurrent access, and waiting for frames.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.

	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.

	std::atomic<time_unit> end_time_{-1}; ///< One after last frame in stream.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
	// can also be computed as end_time_ + 1 - ring_.readable_duration().
	// however this would require mutex lock, because both terms are altered by writer in end_write()
		
	void skip_available_(time_unit duration);
	
public:
	ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t duration);
		
	void initialize();

	time_unit total_duration() const noexcept { return ring_.shape().front(); }

	section_view_type begin_write_span(time_span);
	section_view_type begin_write(time_unit duration);
	void end_write(time_unit written_duration, bool mark_eof = false);

	section_view_type begin_read_span(time_span);
	section_view_type begin_read_span(time_span, bool& reaches_eof);
	section_view_type begin_read(time_unit read_duration);
	section_view_type begin_read(time_unit read_duration, bool& reaches_eof);
	void end_read(time_unit read_duration);	
	
	void skip(time_unit skip_duration);
	void skip_span(time_span);	
	
	time_unit current_time() const noexcept;
	time_unit write_start_time() const noexcept;
	time_unit read_start_time() const noexcept;

	time_unit writable_duration() const;
	time_span writable_time_span() const;
	time_unit readable_duration() const;	
	time_span readable_time_span() const;
	
	/// Returns true if the EOF was marked by the writer.
	/** EOF is marked by writer by an end_write() with \a eof argument set. */
	bool eof_was_marked() const;
	
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_shared_ring.tcc"

#endif
