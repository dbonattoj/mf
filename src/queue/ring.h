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

#ifndef MF_RING_H_
#define MF_RING_H_

#include "../nd/opaque/ndarray_opaque.h"
#include "frame.h"
#include "../utility/misc.h"

namespace mf {

/// Ring buffer.
/** Circular buffer of *frames* of \ref ndarray_opaque_frame_format format.
 ** Derived from 1-dimensional \ref ndarray_opaque. FIFO interface to read/write frames to the ring buffer. */
class ring : private ndarray_opaque<1, raw_ring_allocator> {
	using base = ndarray_opaque<1, raw_ring_allocator>;
	
public:
	using section_view_type = frame_array_view;
	using base::format_base_type;
	using base::format_ptr;

private:
	time_unit read_position_ = 0;
	time_unit write_position_ = 0;
	bool full_ = false;
		
	static std::size_t adjust_padding_(const format_base_type&, std::size_t capacity); 
	section_view_type section_(time_unit start, time_unit duration);

public:
	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	ring(Format&& frm, std::size_t capacity) :
		ring(forward_make_shared_const(frm), capacity) { }
	
	ring(const format_ptr&, std::size_t capacity);
	
	ring(const ring&) = delete;
	ring& operator=(const ring&) = delete;
		
	const format_base_type& frame_format() const noexcept { return base::frame_format(); }
	
	time_unit capacity() const noexcept { return base::shape().front(); }
	
	time_unit writable_duration() const;
	time_unit readable_duration() const;
	
	section_view_type begin_write(time_unit duration);
	void end_write(time_unit written_duration);
	
	section_view_type begin_read(time_unit duration);
	void end_read(time_unit read_duration, bool initialize_frames = true);
		
	void skip(time_unit duration);
};


}

#endif

