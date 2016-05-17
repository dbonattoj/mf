#ifndef MF_TIMED_RING_H_
#define MF_TIMED_RING_H_

#include <atomic>
#include <ostream>
#include <stdexcept>
#include "ring.h"
#include "frame.h"

namespace mf {

/// Ring buffer which adds absolute time index to frames.
class timed_ring : public ring {
	using base = ring;

private:
	/// Time of last written frame.
	/** Defined _current time_ of timed ring buffer. Next write starts at `last_write_time_ + 1`. */
	std::atomic<time_unit> last_write_time_{-1};
	// TODO remove atomic?

public:
	using section_view_type = timed_frame_array_view;

	timed_ring(const frame_array_properties& prop) : base(prop) { }
	
	void initialize();
	
	time_unit current_time() const noexcept { return last_write_time_; }
	time_unit read_start_time() const noexcept;
	time_unit write_start_time() const noexcept;
	
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	bool can_write_span(time_span) const;
	bool can_read_span(time_span) const;
	bool can_skip_span(time_span) const;
	
	section_view_type begin_write(time_unit);
	section_view_type begin_write_span(time_span);
	void end_write(time_unit written_duration);
	
	section_view_type begin_read(time_unit);
	section_view_type begin_read_span(time_span);
	
	void skip_span(time_span);
	
	void seek(time_unit);
};


}

#endif
