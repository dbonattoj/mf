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

#include "ring.h"
#include <stdexcept>
#include <cmath>
#include "../utility/misc.h"
#include "../os/memory.h"

namespace mf {
	

ring::ring(const frame_format& frm, std::size_t capacity) :
	base(frm, capacity, adjust_padding_(frm, capacity)) { }


ring::ring(const frame_array_properties& prop) :
	base(prop, adjust_padding_(prop), raw_ring_allocator()) { }




std::size_t ring::adjust_padding_(const frame_format& frm, std::size_t capacity) {
	std::size_t array_length = capacity; // array length, = number of frames
	std::size_t frame_size = frm.frame_size(); // frame size, in bytes
	std::size_t page_size = system_page_size(); // system page size, in bytes

	std::size_t a = frm.frame_alignment_requirement(); // a = alignment of elements

	// will compute minimal padding frame_padding (bytes to insert between frames)
	// such that array_length * (frame_size + frame_padding) is a multiple of page_size
	//       and (frame_size + frame_padding) is a multiple of a
	
	// frame_size and page_size are necessarily multiples of a, because:
	//    a and page_size are both powers of 2,
	//    page_size >>> a,
	//    frame_size is required to be multiple of a
	// need to make frame_padding also multiple of a
	// --> count in units a
	
	Assert(is_nonzero_multiple_of(frame_size, a), "frame size must be multiple of frame alignment");
	Assert(is_nonzero_multiple_of(page_size, a), "system page size must be multiple of frame alignment");
	
	std::size_t frame_size_a = frame_size / a;
	std::size_t page_size_a = page_size / a;
	// duration * (frame_size_a + frame_padding_a) must be multiple of page_size_a
	
	std::size_t d = page_size_a / gcd(page_size_a, array_length);	
	// --> (frame_size_a + frame_padding_a) must be multiple of d
	// d is a power of 2
	// d = factors 2 that are missing in array_length for it to be multiple of page_size_a
	
	// if frame_size_a is already multiple of d, no padding is needed
	std::size_t r = frame_size_a % d;
	std::size_t frame_padding = 0;
	if(r != 0) frame_padding = (d - r) * a;
	
	Ensures(is_nonzero_multiple_of(prop.array_length() * (frame_size + frame_padding), page_size));
	Ensures(is_nonzero_multiple_of(frame_size + frame_padding, a));
	return frame_padding;
}


void ring::initialize() {
	read_position_ = 0;
	write_position_ = 0;
	full_ = false;
}


auto ring::section_(time_unit start, time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("ring section duration too large");
	
	auto new_start = base::start() + (base::strides().front() * start);
	auto new_shape = make_ndsize(duration, frame_length());
	auto new_strides = base::strides();
	
	return section_view_type(new_start, base::format(), new_shape, new_strides);
}


time_unit ring::readable_duration() const {
	if(full_) return total_duration();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return total_duration() - read_position_ + write_position_;
}


time_unit ring::writable_duration() const {
	return total_duration() - readable_duration();
}


auto ring::begin_write(time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("write duration larger than ring capacity");
	else if(duration > writable_duration()) throw sequencing_error("write duration larger than writable frames");
	else return section_(write_position_, duration);
}


void ring::end_write(time_unit written_duration) {
	if(written_duration == 0) return;
	if(written_duration > writable_duration()) throw std::invalid_argument("reported written duration too large");
	write_position_ = (write_position_ + written_duration) % total_duration();
	if(write_position_ == read_position_) full_ = true;
}
	

auto ring::begin_read(time_unit duration) -> section_view_type {
	if(duration > total_duration()) throw std::invalid_argument("read duration larger than ring capacity");
	else if(duration > readable_duration()) throw sequencing_error("read duration larger than readable frames");
	else return section_(read_position_, duration);
}


void ring::end_read(time_unit read_duration) {
	if(read_duration == 0) return;
	if(read_duration > readable_duration()) throw std::invalid_argument("reported read duration too large");
	read_position_ = (read_position_ + read_duration) % total_duration();
	full_ = false;
}


void ring::skip(time_unit duration) {
	// no check whether duration > total_duration: unlike begin_read/write, the data to skip will not be accessed
	if(duration == 0) return;
	if(duration > readable_duration()) throw std::invalid_argument("skip duration larger than readable frames");
	read_position_ = (read_position_ + duration) % total_duration();
	full_ = false;
}


}
