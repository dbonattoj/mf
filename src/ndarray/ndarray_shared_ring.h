#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>
#include "ndarray_shared_ring_base.h"

namespace mf {

/// Ndarray timed ring for concurrent read/write access.
/** For use with two threads: one which reads and one which writes. Does not support having multiple parallel reads
 ** or multiple parallel writes. Enhancements:
 ** - Mutex lock on buffer state (read and write positions)
 ** - Semantics of begin_write(), begin_read() and skip() changed to wait until frames become available 
 ** - Writer can mark end of file. begin_read(), begin_read_span() and skip() then to not wait for more frames
 ** Does not inherit from base class because thread safety is added. Provides same interface. **/
template<std::size_t Dim, typename T>
class ndarray_shared_ring : public ndarray_shared_ring_base<Dim, T> {
	using base = ndarray_shared_ring_base<Dim, T>;
		
private:
	enum thread_state { idle, waiting, accessing };	

	using typename base::ring_type;
	using typename base::section_view_type;

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
	ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t capacity);
		
	void initialize();

	time_unit capacity() const override { return ring_.shape().front(); }

	section_view_type begin_write(time_unit write_duration) override;
	void end_write(time_unit written_duration) override;
	void end_write(time_unit written_duration, bool mark_end);

	section_view_type begin_read_span(time_span) override;
	section_view_type begin_read(time_unit read_duration) override;
	section_view_type begin_read(time_unit read_duration, bool& reaches_eof);
	section_view_type begin_read_span(time_span, bool& reaches_eof);
	void end_read(time_unit read_duration) override;	
	
	void skip(time_unit skip_duration) override;

	void seek(time_unit) override;
	
	time_unit current_time() const override;
	
	/// True when last frame was written into buffer.
	/** Equivalent to `end_time_is_defined()`, but specific to non-seekable buffer, because end gets marked only by
	 ** end_write() when last frame got written.
	 ** If true, last frame was written but not necessarily read yet. */
	bool writer_reached_end() const { return end_time_is_defined(); }

	/// True when last frame was read from buffer.
	/** Implies `writer_reached_end()`. When true, no more data can be read and begin_read() always returns zero-length
	 ** view. */
	bool reader_reached_end() const { return read_start_time_ == end_time_; }
	

	bool end_time_is_defined() const override { return (end_time_ != -1); }
	time_unit end_time() const override { return end_time_; }

	time_span readable_time_span() const override;
	time_span writable_time_span() const override;
	time_unit read_start_time() const override;
	time_unit write_start_time() const override;

	
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_shared_ring.tcc"

#endif
