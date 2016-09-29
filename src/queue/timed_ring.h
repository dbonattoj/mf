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
	
	class read_handle;
	class write_handle;

private:
	time_unit last_write_time_ = -1;

public:
	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	timed_ring(Format&& frm, std::size_t capacity) :
		timed_ring(forward_make_shared_const(frm), capacity) { }

	timed_ring(const format_ptr&, std::size_t capacity);
		
	const format_base_type& frame_format() const { return ring::frame_format(); }	
	
	time_unit current_time() const;
	time_unit read_start_time() const;
	time_unit write_start_time() const;

	time_unit capacity() const noexcept { return ring::capacity(); }
	time_unit readable_duration() const;
	time_unit writable_duration() const;
	
	time_span readable_time_span() const;
	time_span writable_time_span() const;
	
	write_handle write(time_unit duration) { return write_handle(duration); }
	section_view_type begin_write(time_unit);
	void end_write(time_unit written_duration);
	
	read_handle write(time_unit duration) { return read_handle(duration); }
	read_handle write_span(time_span span) { return read_handle(span); }
	section_view_type begin_read(time_unit);
	section_view_type begin_read_span(time_span);
	void end_read(time_unit read_duration, bool initialize_frames = true);
	void skip(time_unit duration);
	void seek(time_unit);
};


/// Read handle for \ref timed_ring.
class ring::read_handle : public ring_handle_base {
private:
	timed_ring& ring_;
	
public:
	read_handle(timed_ring& rng, time_unit duration) : ring_(rng) {
		reset_view_(ring_.begin_read(duration));
	}
	
	read_handle(timed_ring& rng, time_span span) : ring_(rng) {
		reset_view_(ring_.begin_read_span(span));
	}
	
	~read_handle() override {
		if(! is_null()) end(0);
	}
	
	void end(time_unit duration) override {
		ring_.end_read(duration);
	}
};


/// Write handle of \ref timed_ring.
class ring::write_handle : public ring_handle_base {
private:
	timed_ring& ring_;
	
public:
	write_handle(timed_ring& rng, time_unit duration) : ring_(rng) {
		reset_view_(ring_.begin_write(duration));
	}
	
	~write_handle() override {
		if(! is_null()) end(0);
	}
	
	void end(time_unit duration) override {
		ring_.end_write(duration);
	}
};


}

#endif
