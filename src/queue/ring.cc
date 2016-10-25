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
#include <limits>
#include "../utility/misc.h"
#include "../os/memory.h"

namespace mf {


ring::allocation_parameters ring::select_allocation_parameters_
(const format_base_type& frm, std::size_t min_capacity, std::size_t max_capacity) {
	std::size_t frame_size = frm.frame_size(); // frame size, in bytes
	std::size_t granularity = raw_ring_allocator::size_granularity(); // G = allocated size must be multiple of G
	std::size_t a = frm.frame_alignment_requirement(); // a = alignment of frames

	Assert(is_power_of_two(granularity));
	Assert(is_power_of_two(a));
	Assert(granularity > a);
	Assert(is_nonzero_multiple_of(frame_size, a));

	allocation_parameters param;

	if(is_multiple_of(frame_size, granularity)) {
		std::cerr << "framesize mult of gran" << std::endl;
		
		param.frame_padding = 0;
		param.capacity = min_capacity;
		param.allocated_size = min_capacity * frame_size;
				
	} else if(is_multiple_of(granularity, frame_size)) {
		std::cerr << "gran mult of framesize" << std::endl;
		std::size_t frames_per_chunk = granularity / frame_size;
		
		std::size_t capacity;
		if(is_multiple_of(min_capacity * frame_size, granularity)) capacity = min_capacity;
		else capacity = granularity - ((min_capacity * frame_size) % granularity);
		
		param.frame_padding = 0;
		param.capacity = capacity;
		param.allocated_size = capacity * frame_size;
		
	} else {
		std::cerr << "gen" << std::endl;
		std::size_t frame_size_a = frame_size / a;
		std::size_t granularity_a = granularity / a;
				
		min_capacity = std::max(min_capacity, granularity_a / (2*frame_size_a - 1));
		if(max_capacity == 0) max_capacity = 2 * min_capacity;
		
		std::size_t best_padding_a = frame_size_a;
		std::size_t best_capacity = 0;
		for(std::size_t capacity = min_capacity; capacity <= max_capacity; ++capacity) {
			std::size_t d = granularity_a / gcd(granularity_a, capacity);	
			std::size_t r = frame_size_a % d;
			std::size_t padding_a  = d - r;
			std::cerr << "| cap=" << capacity << ", pad=" << padding_a << std::endl;
			if(padding_a < best_padding_a) {
				best_padding_a = padding_a;
				best_capacity = capacity;
			}
			if(best_padding_a == 1) break;
		}

		param.frame_padding = best_padding_a * a;
		param.capacity = best_capacity;
		param.allocated_size = best_capacity * frame_size;
	}
	
	return param;
}


ring::base ring::make_base_(const format_ptr& frm, allocation_parameters param) {
	return base(make_ndsize(param.capacity), frm, param.frame_padding);
}
	

ring::ring(const format_ptr& frm, std::size_t min_capacity, std::size_t max_capacity) :
	base(make_base_(frm, select_allocation_parameters_(*frm, min_capacity, max_capacity))) { }


auto ring::section_(time_unit start, time_unit duration) -> section_view_type {
	Assert_crit(duration <= capacity());
		
	auto new_start = advance_raw_ptr(base::start(), base::strides().front() * start);
	auto new_shape = make_ndsize(duration);
	auto new_strides = base::strides();
	
	return section_view_type(new_start, new_shape, new_strides, base::frame_format_ptr());
}


time_unit ring::readable_duration() const {
	if(full_) return capacity();
	else if(read_position_ <= write_position_) return write_position_ - read_position_;
	else return capacity() - read_position_ + write_position_;
}


time_unit ring::writable_duration() const {
	return capacity() - readable_duration();
}


auto ring::begin_write(time_unit duration) -> section_view_type {
	if(duration > capacity()) throw std::invalid_argument("write duration larger than ring capacity");
	else if(duration > writable_duration()) throw sequencing_error("write duration larger than writable frames");
	else return section_(write_position_, duration);
}


void ring::end_write(time_unit written_duration) {
	if(written_duration == 0) return;
	if(written_duration > writable_duration()) throw std::invalid_argument("reported written duration too large");
	write_position_ = (write_position_ + written_duration) % capacity();
	if(write_position_ == read_position_) full_ = true;
}
	

auto ring::begin_read(time_unit duration) -> section_view_type {
	if(duration > capacity()) throw std::invalid_argument("read duration larger than ring capacity");
	else if(duration > readable_duration()) throw sequencing_error("read duration larger than readable frames");
	else return section_(read_position_, duration);
}


void ring::end_read(time_unit read_duration, bool initialize_frames) {
	if(read_duration == 0) return;
	Assert_crit(read_duration <= readable_duration());
	if(read_duration > readable_duration()) throw sequencing_error("reported read duration too large");
	if(initialize_frames) {
		auto section = section_(read_position_, read_duration);
		auto it = section.begin(), end_it = section.end();
		for(; it != end_it; ++it) base::initialize_frame(it.ptr());
	}
	read_position_ = (read_position_ + read_duration) % capacity();
	full_ = false;
}


void ring::skip(time_unit duration) {
	// no check whether duration > total_duration: unlike begin_read/write, the data to skip will not be accessed
	if(duration == 0) return;
	if(duration > readable_duration()) throw std::invalid_argument("skip duration larger than readable frames");
	read_position_ = (read_position_ + duration) % capacity();
	full_ = false;
}


}
