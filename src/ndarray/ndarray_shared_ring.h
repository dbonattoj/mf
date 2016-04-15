#ifndef MF_NDARRAY_SHARED_RING_H_
#define MF_NDARRAY_SHARED_RING_H_

#include "ndarray_timed_ring.h"
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_shared_ring {
private:
	/// Indicates current state of reader and writer.
	/** May be `idle` or `accessing`, or a positive integer if thread is waiting for that number of frames. */
	using thread_state = time_unit;
	enum : thread_state {
		idle = 0,
		accessing = -1
	};

	using ring_type = ndarray_timed_ring<Dim, T>;
	using section_view_type = typename ring_type::section_view_type;
	
	const bool seekable_; ///< Whether buffer is seekable or non-seekable, set on construction.

	ring_type ring_; ///< Underlying, non thread-safe timed ring buffer.

	std::atomic<time_unit> end_time_{-1};

	mutable std::mutex mutex_; ///< Protects read/write positions from concurrent access, and waiting for frames.
	std::condition_variable writable_cv_; ///< Condition variable, gets notified when writable frames become available.
	std::condition_variable readable_cv_; ///< Condition variable, gets notified when readable frames become available.


	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
	
	void skip_available_(time_unit duration);
	void read_and_discard_(time_unit duration);
	
public:
	ndarray_shared_ring(const ndsize<Dim>& frames_shape, std::size_t capacity, bool seekable, time_unit end_time_ = -1);
		
	void initialize();

	/// Capacity of buffer.
	/** Maximal readable and writable frames that fit in buffer. */
	time_unit capacity() const { return ring_.shape().front(); }

	/// Begin writing \a duration frames at current write start time.
	/** If span to write crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_write() must not be called.
	 ** For seekable buffer only: returned section may have start time different to `writable_time_span().start_time()`
	 ** when reader seeked to another time inbetween.
	 ** Waits until \a duration (after truncation) frames become writable.
	 ** Returns section for writer to write into, with time information.
	 ** Must be called from single writer thread only, and followed by call to end_write(). */
	section_view_type begin_write(time_unit duration);
	
	/// End writing \a written_duration frames.
	/** Must be called after begin_write(). \a written_duration must be lesser of equal to duration of section returned
	 ** by begin_write().
	 ** The reader cannot seek to other position inbetween begin_write() and end_write() calls of writer. */
	void end_write(time_unit written_duration, bool mark_end = false);

	/// Begin reading frames at time span \a span.
	/** If \a span does not start at `readable_time_span().start_time()`, seeks to `span.start_time()` first.
	 ** Then behaves as `begin_read(span.duration())`.  */
	section_view_type begin_read_span(time_span);
	
	/// Begin reading \a duration frames at current read start time.
	/** If span to read crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_read() must not be called.
	 ** Waits until \a duration (after truncation) frames become readable.
	 ** For non-seekable buffer only: end may be marked while waiting, and so returned section may be truncated even if
	 ** end time was not defined prior to call.
	 ** Returns section for writer to read from, with time information. Reader may also freely write into the returned
	 ** section.
	 ** Must be called from single reader thread only, and followed by call to end_read(). */
	section_view_type begin_read(time_unit read_duration);
	
	/// End reading \a read_duration frames.
	/** Must be called after begin_read() or begin_read_span(). \a read_duration must be lesser of equal to duration
	 ** of section returned by that function. */
	void end_read(time_unit read_duration);	
	
	
	/// Skips \a duration frames.
	/** If span to skip crosses end of buffer, it is truncated, and skips to end of file.
	 ** \a duration can be larger than buffer capacity.
	 ** For non-seekable buffer, reads and discards \a duration frames, possibly letting writer fill up buffer multiple
	 ** times. For seekable buffer, equivalent to `seek(read_start_time() + duration)`.
	 ** Must be called from single reader thread only. */
	void skip(time_unit skip_duration);
	
	
	bool is_seekable() const { return seekable_; }
	
	/// Seeks to read time \a t.
	/** Can only be called on seekable buffer. Must be called from single reader thread only. 
	 ** Sets read start position to absolute time \a t, producing internal discontinuity in buffer. Change is signalled
	 ** to writer by the section returned by `begin_write()`, and writer must react by writing frames for that time. */
	void seek(time_unit target_time);
	
	
	/// Verifies if is is possible to seek to read time \a t.
	/** Returns false when buffer is not seekable, or when time is out of bounds. */
	bool can_seek(time_unit target_time) const;
	
	/// Time of last written frame in buffer.
	/** Equivalent to `write_start_time() - 1`. -1 in initial state. */
	time_unit current_time() const;
	
	time_unit write_start_time() const;
	time_unit read_start_time() const;
	
	[[deprecated]] time_unit writable_duration() const { return writable_time_span().duration(); }
	[[deprecated]] time_unit readable_duration() const { return readable_time_span().duration(); }
	
	time_span writable_time_span() const;
	time_span readable_time_span() const;
	
		
	/// End of file time. */
	/** Returns -1 if not defined. For non-seekable buffer, reader must not rely on this value because it can change
	 ** between end_time() call and begin_read() call. */
	time_unit end_time() const { return end_time_; }
	
	/// True if end of file time is known. */
	bool end_time_is_defined() const { return (end_time_ != -1); }
	
	bool writer_reached_end() const;
	bool reader_reached_end() const;
		
	#ifndef NDEBUG
	void debug_print(std::ostream&) const;
	#endif
};

}

#include "ndarray_shared_ring.tcc"

#endif
