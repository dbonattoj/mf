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

#include <stdexcept>
#include "ring.h"
#include "frame.h"
#include "../utility/misc.h"

namespace mf {

/// Ring buffer which adds absolute time index to frames.
/** Not thread-safe, member functions should not be called from multiple threads concurrently. */
class timed_ring : private ring {
public:
	using section_view_type = timed_frame_array_view;
	using ring::format_base_type;
	using ring::format_ptr;

	static constexpr time_unit undefined_time = -1;

private:
	time_unit last_write_time_ = -1;
	time_unit end_time_ = undefined_time;

public:
	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	timed_ring(Format&& frm, std::size_t capacity, time_unit end_time = undefined_time) :
		timed_ring(forward_make_shared_const(frm), capacity, end_time) { }

	timed_ring(const format_ptr&, std::size_t capacity, time_unit end_time);
		
	const format_base_type& frame_format() const { return ring::frame_format(); }	
	time_unit end_time() const { return end_time_; }
	
	time_unit current_time() const;
	time_unit read_start_time() const;
	time_unit write_start_time() const;

	time_unit capacity() const noexcept { return ring::capacity(); }
	time_unit readable_duration() const;
	time_unit writable_duration() const;
	
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	section_view_type begin_write(time_unit);
	void end_write(time_unit written_duration);
	bool writer_reached_end() const;
	
	section_view_type begin_read(time_unit);
	section_view_type begin_read_span(time_span);
	void end_read(time_unit read_duration, bool initialize_frames);
	void skip(time_unit duration);
	void seek(time_unit);
	bool reader_reached_end() const;
};


}

#endif
