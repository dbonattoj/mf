#ifndef MF_SHARED_RING_H_
#define MF_SHARED_RING_H_

#include "timed_ring.h"
#include "../utility/event.h"
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>

namespace mf {

class shared_ring {
public:
	using section_view_type = timed_ring::section_view_type;

private:
	/// Indicates current state of reader and writer.
	/** May be `idle` or `accessing`, or a positive integer if thread is waiting for that number of frames. */
	using thread_state = time_unit;
	enum : thread_state {
		idle = 0,
		accessing = -1
	};

	timed_ring ring_; ///< Underlying, non thread-safe timed ring buffer.

	const bool seekable_;

	std::atomic<time_unit> end_time_{-1};
	// TODO make non-atomic? verify

	mutable std::mutex mutex_; ///< Protects read/write positions from concurrent access.
	event reader_idle_event_; ///< Event sent by reader when is becomes idle, and received by writer.
	event reader_seek_event_; ///< Event sent by reader after is has seeked, and changed write start time.
	event writer_idle_event_; ///< Event sent by writer when is becomes idle, and received by reader.

	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
		
	void skip_available_(time_unit duration);
	void read_and_discard_(time_unit duration);
	
public:
	shared_ring(const frame_properties&, std::size_t capacity, bool seekable, time_unit end_time = -1);
		
	void initialize();

	/// Capacity of buffer.
	/** Maximal readable and writable frames that fit in buffer. */
	time_unit capacity() const { return ring_.shape().front(); }

	/// Begin writing \a write_duration frames at current write start time.
	/** If span to write crosses end of buffer, it is truncated.
	 ** For seekable buffer only: returned section may have start time different to `writable_time_span().start_time()`
	 ** when reader seeked to another time inbetween.
	 ** When at end (already before, or following wait and seek), zero-length section is returned, and start time of
	 ** returned time span equals `end_time()`.
	 ** Then end_write() must not be called.
	 ** Waits until \a write_duration (after truncation) frames become writable.
	 ** Returns section for writer to write into, with time information.
	 ** Must be called from single writer thread only, and followed by call to end_write(). */
	section_view_type begin_write(time_unit write_duration);
	
	/// Begin writing \a write_duration frames at current write start time, if possible.
	/** For seekable buffer only: returned section may have start time different to `writable_time_span().start_time()`
	 ** when reader seeked to another time inbetween. If span to write crosses end of buffer, it is truncated first.
	 ** If that span is not writable because not enough writable frames are available, returns false, and does not wait.
	 ** Otherwise returns true and puts writable section in \a section. */
	bool try_begin_write(time_unit write_duration, section_view_type& section);

	/// Wait until a frame become writable, or \a break_event occurs.
	/** If no frame is writable (either because reader has not read enough frames from the ring buffer yet or write
	 ** start time is at end), waits until at least one frame becomes available, or \a break_event occurs.
	 ** If \a break_event occured, returns false. Otherwise, repeats until at least one frames is writable.
	 ** Also waits if write start position is at end time, until seek occurs or \a break_event occurs. */
	bool wait_writable(event& break_event);
	
	/// End writing \a written_duration frames.
	/** Must be called after begin_write(). \a written_duration must be lesser of equal to duration of section returned
	 ** by begin_write(). For non-seekable buffer, \a mark_end is used to mark this written frame(s) as last.
	 ** \a mark_end cannot be set when \a written_duration is zero.
	 ** The reader cannot seek to other position inbetween begin_write() and end_write() calls of writer. */
	void end_write(time_unit written_duration, bool mark_end = false);

	/// Begin reading frames at time span \a span.
	/** If \a span does not start at `readable_time_span().start_time()`, seeks to `span.start_time()` first.
	 ** Then behaves as `begin_read(span.duration())`.  */
	section_view_type begin_read_span(time_span);
	
	/// Begin reading \a read_duration frames at current read start time.
	/** If span to read crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_read() must not be called.
	 ** Waits until \a read_duration (after truncation) frames become readable.
	 ** For non-seekable buffer only: end may be marked while waiting, and so returned section may be truncated even if
	 ** end time was not defined prior to call.
	 ** Returns section for writer to read from, with time information. Reader may also freely write into the returned
	 ** section.
	 ** Must be called from single reader thread only, and followed by call to end_read(). */
	section_view_type begin_read(time_unit read_duration);


	bool try_begin_read(time_unit read_duration, section_view_type& section);

	/// Wait until a frame become readable, or \a break_event occurs.
	/** If no frame is readable (either because writer has not written enough frames from the ring buffer yet or read
	 ** start time is at end), waits until at least one frame becomes available, or \a break_event occurs.
	 ** If \a break_event occured, returns false. Otherwise, repeats until at least one frames is readable.
	 ** Also waits if write start position is at end time, until \a break_event occurs. */
	bool wait_readable(event& break_event);

	
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
	
	/// True if writer has written last frame.
	/** For non-seekable buffer, true after end_write() call with mark end. begin_write() returns empty view if called
	 ** after this returned true. (Except if seek occured inbetween). */
	bool writer_reached_end() const;

	/// True if reader has read last frame.
	/** Unlike end_time(), the result cannot change between reader_reached_end() and begin_read() call: for non-seekable
	 ** buffer, the writer can only mark end after writing at least one frame, and hence cannot retroactively mark
	 ** current read start position as being the end. */
	bool reader_reached_end() const;
};


}

#endif