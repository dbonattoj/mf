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

#include "ndarray_opaque_frame_format.h"
#include "../../utility/misc.h"
#include <cstring>

namespace mf {
	
ndarray_opaque_frame_format::ndarray_opaque_frame_format() :
	frame_size_(0),
	frame_alignment_requirement_(1),
	contiguous_(true) { }

	
ndarray_opaque_frame_format::ndarray_opaque_frame_format(const ndarray_format& format) {
	parts_.push_back({ 0, format });
	frame_size_ = format.frame_size();
	frame_alignment_requirement_ = format.frame_alignment_requirement();
	contiguous_ = format.is_contiguous();
}


ndarray_opaque_frame_format::ndarray_opaque_frame_format(std::size_t frame_size, std::size_t frame_alignment_req) :
	frame_size_(frame_size),
	frame_alignment_requirement_(frame_alignment_req),
	contiguous_(true) { }


auto ndarray_opaque_frame_format::add_part(const ndarray_format& new_part_format) -> const part& {
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
	frame_size_ = new_part.offset + new_part.format.frame_size();
	frame_alignment_requirement_ = lcm(frame_alignment_requirement_, new_part.format.frame_alignment_requirement());
	return parts_.back();
}
	

bool operator==(const ndarray_opaque_frame_format& a, const ndarray_opaque_frame_format& b) {
	return
		(a.parts_ == b.parts_) &&
		(a.frame_size_ == b.frame_size_) &&
		(a.frame_alignment_requirement_ == b.frame_alignment_requirement_);
}


bool operator!=(const ndarray_opaque_frame_format& a, const ndarray_opaque_frame_format& b) {
	return
		(a.parts_ != b.parts_) ||
		(a.frame_size_ != b.frame_size_) ||
		(a.frame_alignment_requirement_ != b.frame_alignment_requirement_);
}


bool ndarray_opaque_frame_compare(const void* a, const void* b, const ndarray_opaque_frame_format& format) {
	if(format.is_contiguous()) {
		return (std::memcmp(a, b, format.frame_size()) == 0);
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < format.parts_count(); ++part_index) {
			const auto& part = format.part_at(part_index);
			bool part_equal = ndarray_data_compare(
				advance_raw_ptr(a, part.offset),
				advance_raw_ptr(b, part.offset),
				part.format
			);
			if(!part_equal) return false;
		}
		return true;
	}
}


void ndarray_opaque_frame_copy(void* destination, const void* origin, const ndarray_opaque_frame_format& format) {
	if(format.is_contiguous()) {
		std::memcpy(destination, origin, format.frame_size());
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < format.parts_count(); ++part_index) {
			const auto& part = format.part_at(part_index);
			ndarray_data_copy(
				advance_raw_ptr(destination, part.offset),
				advance_raw_ptr(origin, part.offset),
				part.format
			);
		}
	}

}



}
