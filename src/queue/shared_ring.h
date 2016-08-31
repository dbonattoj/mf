/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_SHARED_RING_H_
#define MF_SHARED_RING_H_

#include "timed_ring.h"
#include <mutex>
#include <condition_variable>
#include <stdexcept>
#include <atomic>
#include <ostream>
#include <tuple>
#include <utility>

namespace mf {

/// Timed ring buffer with changed semantics, for dual-thread use.
/** Meant to be accessed from two threads, one reader and one writer.
 ** Changed semantics:
 ** - When trying to read/write more than readable/writable duration, will block until the frames become available
 **   from the other thread. Deadlocks are prevented.
 ** - If span to read, write, skip crosses end time, it gets truncated. */
class shared_ring {
public:
	using section_view_type = timed_ring::section_view_type;
	using format_base_type = timed_ring::format_base_type;
	using format_ptr = timed_ring::format_ptr;

	static constexpr time_unit undefined_time = timed_ring::undefined_time;

private:
	/// Indicates current state of reader and writer.
	/** May be `idle` or `accessing`, or a positive integer if thread is waiting for that number of frames. */
	using thread_state = time_unit;
	enum : thread_state { idle = 0, accessing = -1 };

	timed_ring ring_;

	mutable std::mutex mutex_; ///< Protects timed_ring from concurrent access, and 
	std::condition_variable readable_cv_;
	std::condition_variable writable_cv_;
	
	std::atomic_flag reader_keep_waiting_ = ATOMIC_FLAG_INIT;
	std::atomic_flag writer_keep_waiting_ = ATOMIC_FLAG_INIT;

	std::atomic<thread_state> reader_state_{idle}; ///< Current state of reader thread. Used to prevent deadlocks.
	std::atomic<thread_state> writer_state_{idle}; ///< Current state of writer thread. Used to prevent deadlocks.
	
	std::atomic<time_unit> read_start_time_{0}; ///< Absolute time corresponding to current read start time.
		
	bool reader_is_waiting_() const { return (reader_state_ > 0); }
	bool writer_is_waiting_() const { return (writer_state_ > 0); }
	time_unit frames_reader_waits_for_() const { return std::max<time_unit>(reader_state_.load(), 0); }
	time_unit frames_writer_waits_for_() const { return std::max<time_unit>(writer_state_.load(), 0); }

	shared_ring(const format_ptr& frm, std::size_t capacity, time_unit end_time);

public:
	template<typename Format>
	shared_ring(Format&& frm, std::size_t capacity, time_unit end_time = undefined_time) :
		shared_ring(forward_make_shared(frm), capacity, end_time) { }
			
	void break_reader();
	void break_writer();
	
	
	const format_base_type& frame_format() const noexcept { return ring_.frame_format(); }

	/// Capacity of buffer.
	time_unit capacity() const { return ring_.capacity(); }

	/// \name Write interface.

	/// Must be called by the writer thread only.
	///@{
	
	/// Wait until a frame become writable, or writer break event occurs.
	/** If no frame is writable (either because reader has not read enough frames yet or write start time is at end),
	 ** waits until at least one frame becomes available, or break_writer() was called, triggering writer break event.
	 ** If \a break_event occured, returns `false`. Otherwise returns `true`.
	 ** Also waits if write start position is at end time, until seek occurs or writer break event occurs. */
	bool wait_writable();
	
	/// Begin writing \a write_duration frames at current write start time, if they are available.
	/** Like begin_write(), but never waits. Instead returns null view when the frames are not available. */
	section_view_type try_begin_write(time_unit write_duration);

	/// Begin writing \a write_duration frames at current write start time.
	/** If span to write crosses end of buffer, it is truncated.
	 ** Waits until \a write_duration (after truncation) frames become writable.
	 ** When at end, zero-length section is returned, and start time of returned time span equals `end_time()`.
	 ** Then end_write() need not be called.
	 ** Otherwise, returns section for writer to write into, with the correct start time.
	 ** Returned section may have start time different to `writable_time_span().start_time()` called just before,
	 ** when reader seeked to another time in-between the calls.
	 ** Must be followed by call to end_write(). */
	section_view_type begin_write(time_unit write_duration);
		
	/// End writing \a did_write_duration frames.
	/** Must be called after begin_write(). \a did_write_duration must be lower of equal to duration of section returned
	 ** by begin_write(). */
	void end_write(time_unit did_write_duration);

	///@}


	/// \name Read interface.
	/// Must be called by the reader thread only.
	///@{

	/// Wait until a frame become readable, or reader break event occurs.
	/** If no frame is readable (either because writer has not written enough frames yet or read start time is at end), 
	 ** waits until at least one frame becomes available, or break_reader() was called, triggering reader break event.
	 ** If reader break event occured, returns `false`. Otherwise returns `true`.
	 ** Also waits if write start position is at end time, until reader break event occurs. */
	bool wait_readable();

	/// Begin reading frames at time span \a span.
	/** If \a span does not start at `readable_time_span().start_time()`, seeks to `span.start_time()` first.
	 ** Then behaves as `begin_read(span.duration())`.  */
	section_view_type begin_read_span(time_span span);
	
	/// Begin reading \a read_duration frames at current read start time.
	/** If span to read crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_read() must not be called.
	 ** Waits until \a read_duration (after truncation) frames become readable.
	 ** Returns section for writer to read from.
	 ** Must be followed by call to end_read(). */
	section_view_type begin_read(time_unit read_duration);

	/// Begin reading \a read_duration frames at current read start time, if they are available.
	/** Like begin_read(), but never waits. Instead returns null view when the frames are not available. */
	section_view_type try_begin_read(time_unit read_duration);
	
	/// End reading \a did_read_duration frames.
	/** Must be called after begin_read() or begin_read_span(). \a did_read_duration must be lower or equal to duration
	 ** of section returned by that function. */
	void end_read(time_unit did_read_duration);	
	
	/// Skips \a skip_duration frames.
	/** Equivalent to `seek(read_start_time() + skip_duration)`, except that \a skip_duration gets truncated if it
	 ** crosses the to end time. */
	void skip(time_unit skip_duration);
	
	/// Seeks to read time \a target_time.
	/** Sets read start position to absolute time \a target_time, producing internal discontinuity in buffer.
	 ** Change is signalled to writer by the start time of the timed section returned by `begin_write()`,
	 ** and writer will react by writing frames for that time. */
	void seek(time_unit target_time);
	
	/// Verifies if is is possible to seek to read time \a target_time.
	bool can_seek(time_unit target_time) const;
	
	///@}
	
	/// Presumptive start time of next write.
	/** Value gets changed by writer, but can also be changed by reader using seek(). The change can occur during a
	 ** begin_write() of the writer while it is waiting. Writer must always use start time of section returned by
	 ** begin_write() or try_begin_write() instead. */
	time_unit write_start_time() const;
	
	/// Read start time.
	/** Value gets changed by reader. Start time of section returned by next call to begin_read(). */
	time_unit read_start_time() const;
		
	time_span writable_time_span() const;
	
	time_span readable_time_span() const;
	
	time_unit writable_duration() const { return writable_time_span().duration(); }

	time_unit readable_duration() const { return readable_time_span().duration(); }

	/// End of file time. */
	time_unit end_time() const { return ring_.end_time(); }
		
	/// True if writer has written last frame.
	/** For non-seekable buffer, true after end_write() call with mark end. begin_write() returns empty view if called
	 ** after this returned true. (Except if seek occured inbetween). */
	bool writer_reached_end() const;

	/// True if reader has read last frame.
	/** Unlike end_time(), the result cannot change between reader_reached_end() and begin_read() call: for non-seekable
	 ** buffer, the writer can only mark end after writing at least one frame, and hence cannot retroactively mark
	 ** current read start position as being the end. */
	bool reader_reached_end() const;
	
	time_unit current_time() const { return ring_.current_time(); }
};


}

#endif
