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

	time_unit readable_duration() const { return readable_time_span().duration(); }
	time_unit writable_duration() const { return writable_time_span().duration(); } 
	
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
