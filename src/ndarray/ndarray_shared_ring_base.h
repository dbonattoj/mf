#ifndef MF_NDARRAY_SHARED_RING_BASE_H_
#define MF_NDARRAY_SHARED_RING_BASE_H_

#include "ndarray_timed_ring.h"

namespace mf {

template<std::size_t Dim, typename T>
class ndarray_shared_ring_base {	
public:
	using ring_type = ndarray_timed_ring<Dim, T>;
	using section_view_type = typename ring_type::section_view_type;

	virtual ~ndarray_shared_ring_base() { }

	/// Begin writing \a duration frames at current write start time.
	/** If span to write crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_write() must not be called.
	 ** For seekable buffer only: returned section may have start time different to `writable_time_span().start_time()`
	 ** when reader seeked to another time inbetween.
	 ** Waits until \a duration (after truncation) frames become writable.
	 ** Returns section for writer to write into, with time information.
	 ** Must be called from single writer thread only, and followed by call to end_write(). */
	virtual section_view_type begin_write(time_unit duration) = 0;
	
	/// End writing \a written_duration frames.
	/** Must be called after begin_write(). \a written_duration must be lesser of equal to duration of section returned
	 ** by begin_write().
	 ** The reader cannot seek to other position inbetween begin_write() and end_write() calls of writer. */
	virtual void end_write(time_unit written_duration) = 0;

	/// Begin reading \a duration frames at current read start time.
	/** If span to read crosses end of buffer, it is truncated. When already at end, zero-length section is returned.
	 ** Then end_read() must not be called.
	 ** Waits until \a duration (after truncation) frames become readable.
	 ** For non-seekable buffer only: end may be marked while waiting, and so returned section may be truncated even if
	 ** end time was not defined prior to call.
	 ** Returns section for writer to read from, with time information. Reader may also freely write into the returned
	 ** section.
	 ** Must be called from single reader thread only, and followed by call to end_read(). */
	virtual section_view_type begin_read(time_unit duration) = 0;
	
	/// Begin reading frames at time span \a span.
	/** If \a span does not start at `readable_time_span().start_time()`, seeks to `span.start_time()` first.
	 ** Then behaves as `begin_read(span.duration())`.  */
	virtual section_view_type begin_read_span(time_span span) = 0;
	
	/// End reading \a read_duration frames.
	/** Must be called after begin_read() or begin_read_span(). \a read_duration must be lesser of equal to duration
	 ** of section returned by that function. */
	virtual void end_read(time_unit read_duration) = 0;
	
	/// Skips \a duration frames.
	/** If span to skip crosses end of buffer, it is truncated, and skips to end of file.
	 ** \a duration can be larger than buffer capacity.
	 ** For non-seekable buffer, reads and discards \a duration frames, possibly letting writer fill up buffer multiple
	 ** times. For seekable buffer, equivalent to `seek(read_start_time() + duration)`.
	 ** Must be called from single reader thread only. */
	virtual void skip(time_unit duration) = 0;
	
	/// Seeks to read time \a t.
	/** Can only be called on seekable buffer. Must be called from single reader thread only. 
	 ** Sets read start position to absolute time \a t, producing internal discontinuity in buffer. Change is signalled
	 ** to writer by the section returned by `begin_write()`, and writer must react by writing frames for that time. */
	virtual void seek(time_unit t) = 0;
	
	/// Verifies if is is possible to seek to read time \a t.
	/** Returns false when buffer is not seekable, or when time is out of bounds. */
	virtual bool can_seek(time_unit t) const = 0;
	
	/// Capacity of buffer.
	/** Maximal readable and writable frames that fit in buffer. */
	virtual time_unit capacity() const = 0;
	
	/// True if end of file time is known. */
	virtual bool end_time_is_defined() const = 0;
	
	/// End of file time. */
	/** Returns -1 if not defined. For non-seekable buffer, reader must not rely on this value because it can change
	 ** between end_time() call and begin_read() call. */
	virtual time_unit end_time() const = 0;
	
	/// Time of last written frame in buffer.
	/** Equivalent to `write_start_time() - 1`. -1 in initial state. */
	virtual time_unit current_time() const = 0;
	
	/// True when last frame was written to buffer.
	virtual bool writer_reached_end() const = 0;

	/// True when last frame was read from buffer.
	virtual bool reader_reached_end() const = 0;

	
	virtual time_span readable_time_span() const = 0;
	virtual time_span writable_time_span() const = 0;
	virtual time_unit read_start_time() const { return this->readable_time_span().start_time(); }
	virtual time_unit write_start_time() const { return this->writable_time_span().start_time(); }
	time_unit readable_duration() const { return this->readable_time_span().duration(); }
	time_unit writable_duration() const { return this->writable_time_span().duration(); }
	
	#ifndef NDEBUG
	virtual void debug_print(std::ostream&) const = 0;
	#endif
};

}

#endif
