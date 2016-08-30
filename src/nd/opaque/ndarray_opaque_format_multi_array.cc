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

#include "ndarray_opaque_format_multi_array.h"
#include "../../utility/misc.h"
#include <cstring>

namespace mf {
	
ndarray_opaque_format_multi_array::ndarray_opaque_format_multi_array() :
	frame_size_without_end_padding_(0),
	frame_alignment_requirement_(1),
	frame_size_with_end_padding_(0),
	contiguous_(true) { }


void ndarray_opaque_format_multi_array::update_frame_size_with_end_padding_() {
	if(is_multiple_of(frame_size_without_end_padding_, frame_alignment_requirement_)) {
		frame_size_with_end_padding_ = frame_size_without_end_padding_;
	} else {
		std::size_t remainder = frame_size_without_end_padding_ % frame_alignment_requirement_;
		frame_size_with_end_padding_ = frame_size_without_end_padding_ + (frame_alignment_requirement_ - remainder);
	}
	Assert(is_multiple_of(frame_size_with_end_padding_, frame_alignment_requirement_));
}


auto ndarray_opaque_format_multi_array::add_part(const ndarray_format& new_part_format) -> const part& {
	part new_part { 0, new_part_format };
	
	if(parts_.size() > 0) {
		const part& previous_part = parts_.back();
		std::ptrdiff_t min_offset = previous_part.offset + previous_part.format.frame_size();
		std::ptrdiff_t alignment_requirement = new_part_format.frame_alignment_requirement();
				
		if(is_multiple_of(min_offset, alignment_requirement)) {
			new_part.offset = min_offset;
		} else {
			new_part.offset = (1 + (min_offset / alignment_requirement)) * alignment_requirement;
			contiguous_ = false;
		}

		Assert(is_multiple_of(new_part.offset, alignment_requirement));
		Assert(new_part.offset >= min_offset);
	}
	
	parts_.push_back(new_part);
	frame_size_without_end_padding_ = new_part.offset + new_part.format.frame_size();
	frame_alignment_requirement_ = lcm(frame_alignment_requirement_, new_part.format.frame_alignment_requirement());
	update_frame_size_with_end_padding_();
	return parts_.back();
}
	

bool operator==(const ndarray_opaque_format_multi_array& a, const ndarray_opaque_format_multi_array& b) {
	return
		(a.parts_ == b.parts_) &&
		(a.frame_size_without_end_padding_ == b.frame_size_without_end_padding_) &&
		(a.frame_alignment_requirement_ == b.frame_alignment_requirement_);
}


bool operator!=(const ndarray_opaque_format_multi_array& a, const ndarray_opaque_format_multi_array& b) {
	return
		(a.parts_ != b.parts_) ||
		(a.frame_size_without_end_padding_ != b.frame_size_without_end_padding_) ||
		(a.frame_alignment_requirement_ != b.frame_alignment_requirement_); // TODO check (only parts enough?)
}



void ndarray_opaque_format_multi_array::copy_frame(frame_ptr destination, const_frame_ptr origin) const {
	if(is_contiguous()) {
		std::memcpy(destination, origin, frame_size());
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < parts_count(); ++part_index) {
			const auto& prt = part_at(part_index);
			ndarray_data_copy(
				advance_raw_ptr(destination, prt.offset),
				advance_raw_ptr(origin, prt.offset),
				prt.format
			);
		}
	}
}
	

bool ndarray_opaque_format_multi_array::compare_frame(const_frame_ptr a, const_frame_ptr b) const {
	if(is_contiguous()) {
		return (std::memcmp(a, b, frame_size()) == 0);
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < parts_count(); ++part_index) {
			const auto& prt = part_at(part_index);
			bool part_equal = ndarray_data_compare(
				advance_raw_ptr(a, prt.offset),
				advance_raw_ptr(b, prt.offset),
				prt.format
			);
			if(!part_equal) return false;
		}
		return true;
	}
}
	

void ndarray_opaque_format_multi_array::construct_frame(frame_ptr) const { }

	
void ndarray_opaque_format_multi_array::destruct_frame(frame_ptr) const { }


}
