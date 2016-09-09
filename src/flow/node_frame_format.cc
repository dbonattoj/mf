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

#include "node_frame_format.h"
#include "node_frame_satellite.h"
#include "../utility/misc.h"

namespace mf { namespace flow {

namespace {
	constexpr std::size_t satellite_size_ = sizeof(node_frame_satellite);
	constexpr std::size_t satellite_alignment_requirement_ = alignof(node_frame_satellite);
}

node_frame_format::node_frame_format() :
	base(),
	satellite_offset_(0)
{
	// set up space for pointer only
	set_frame_size_(satellite_size_);
	set_frame_alignment_requirement_(satellite_alignment_requirement_);
	// always non-pod because of the pointer
	set_pod_(false);
	set_contiguous_(false);
}


node_frame_satellite* node_frame_format::frame_satellite_ptr(frame_ptr frame) const {
	auto raw_ptr = advance_raw_ptr(frame, satellite_offset_);
	return reinterpret_cast<node_frame_satellite*>(raw_ptr);
}


const node_frame_satellite* node_frame_format::frame_satellite_ptr(const_frame_ptr frame) const {
	auto raw_ptr = advance_raw_ptr(frame, satellite_offset_);
	return reinterpret_cast<const node_frame_satellite*>(raw_ptr);
}


node_frame_satellite& node_frame_format::frame_satellite(frame_ptr frame) const {
	return *frame_satellite_ptr(frame);
}


const node_frame_satellite& node_frame_format::frame_satellite(const_frame_ptr frame) const {
	return *frame_satellite_ptr(frame);
}


void node_frame_format::readjust_for_added_part_(const part& new_part, bool has_padding) {	
	// calculate frame alignment
	// old_frame_alignment_requirement already includes that for satellite
	std::size_t old_frame_alignment_requirement = frame_alignment_requirement();
	set_frame_alignment_requirement_(lcm(
		old_frame_alignment_requirement,
		new_part.format.frame_alignment_requirement())
	);

	// calculate padding between last part and satellite, and satellite offset
	std::size_t parts_size_without_end_padding = new_part.offset + new_part.format.frame_size();
	std::size_t padding_before_satellite;
	
	if(is_multiple_of(parts_size_without_end_padding, satellite_alignment_requirement_)) {
		// no padding needed
		padding_before_satellite = 0;
	} else {
		// need padding between parts and pointer
		std::size_t remainder = parts_size_without_end_padding % satellite_alignment_requirement_;
		padding_before_satellite = parts_size_without_end_padding + (satellite_alignment_requirement_ - remainder);
	}
	
	satellite_offset_ = parts_size_without_end_padding + padding_before_satellite;

	// calculate full frame size, with additional padding at end if required
	std::size_t size_without_end_padding = parts_size_without_end_padding + padding_before_satellite + satellite_size_;
	std::size_t size_with_end_padding = frame_size_with_end_padding_(size_without_end_padding);

	set_frame_size_(size_with_end_padding);
	
	Assert(is_multiple_of(satellite_offset_, satellite_alignment_requirement_), "satellite must be aligned");
	Assert(size_with_end_padding - satellite_offset_ >= satellite_size_, "must have enough space for satellite");
	Assert(is_multiple_of(size_with_end_padding, frame_alignment_requirement()), "frame size must fit alignment");
}


void node_frame_format::copy_frame(frame_ptr destination, const_frame_ptr origin) const {
	base::copy_frame(destination, origin);
	frame_satellite(destination) = frame_satellite(origin);
}


bool node_frame_format::compare_frame(const_frame_ptr a, const_frame_ptr b) const {
	return (base::compare_frame(a, b)) && (frame_satellite(a) == frame_satellite(b));
}


void node_frame_format::construct_frame(frame_ptr frame) const {
	void* ptr = frame_satellite_ptr(frame);
	new(ptr) node_frame_satellite();
}


void node_frame_format::destruct_frame(frame_ptr frame) const {
	frame_satellite_ptr(frame)->~node_frame_satellite();
}


void node_frame_format::initialize_frame(frame_ptr frame) const {
	frame_satellite(frame).initialize();
}


bool node_frame_format::has_array_format() const {
	return (parts_count() == 1);
}


ndarray_format node_frame_format::array_format() const {
	Assert(has_array_format(), "can only get array_format of node_frame_format that has only 1 part (channel)");
	Assert(part_at(0).offset == 0);
	return part_at(0).format;
}


}}
