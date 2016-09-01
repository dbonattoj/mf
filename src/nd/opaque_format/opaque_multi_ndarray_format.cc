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

#include "opaque_multi_ndarray_format.h"
#include "opaque_ndarray_format.h"
#include <cstring>

namespace mf {
	
bool opaque_multi_ndarray_format::compare(const opaque_format& frm) const {
	if(typeid(opaque_multi_ndarray_format) == typeid(frm))
		return (static_cast<const opaque_multi_ndarray_format&>(frm) == *this);
	else
		return false;
}


std::size_t opaque_multi_ndarray_format::frame_size_with_end_padding_(std::size_t without_end_padding) const {
	const std::size_t alignment_requirement = frame_alignment_requirement();
	std::size_t with_end_padding = 0;
	if(is_multiple_of(without_end_padding, alignment_requirement)) {
		with_end_padding = without_end_padding;
	} else {
		std::size_t remainder = without_end_padding % alignment_requirement;
		with_end_padding = without_end_padding + (alignment_requirement - remainder);
	}
	Assert(is_multiple_of(with_end_padding, alignment_requirement));
	return with_end_padding;
}


opaque_multi_ndarray_format::opaque_multi_ndarray_format() {
	set_frame_size_(0);
	set_frame_alignment_requirement_(1);
	set_pod_(true);
	set_contiguous_(true);
}


auto opaque_multi_ndarray_format::add_part(const ndarray_format& new_part_format) -> const part& {
	part new_part { new_part_format, 0 };
	bool has_padding = false;
	
	if(parts_.size() > 0) {
		const part& previous_part = parts_.back();
		std::ptrdiff_t min_offset = previous_part.offset + previous_part.format.frame_size();
		std::ptrdiff_t alignment_requirement = new_part_format.frame_alignment_requirement();
				
		if(is_multiple_of(min_offset, alignment_requirement)) {
			new_part.offset = min_offset;
		} else {
			new_part.offset = (1 + (min_offset / alignment_requirement)) * alignment_requirement;
			has_padding = true;
		}

		Assert(is_multiple_of(new_part.offset, alignment_requirement));
		Assert(new_part.offset >= min_offset);
	}
	
	parts_.push_back(new_part);
	
	this->readjust_for_added_part_(new_part, has_padding);
			
	return parts_.back();
}


void opaque_multi_ndarray_format::readjust_for_added_part_(const part& new_part, bool has_padding) {
	if(has_padding) set_contiguous_(false);
	
	std::size_t old_frame_alignment_requirement = frame_alignment_requirement();
	set_frame_alignment_requirement_(lcm(
		old_frame_alignment_requirement,
		new_part.format.frame_alignment_requirement())
	);

	std::size_t frame_size_without_end_padding = new_part.offset + new_part.format.frame_size();
	std::size_t frame_size_with_end_padding = frame_size_with_end_padding_(frame_size_without_end_padding);
	set_frame_size_(frame_size_with_end_padding);
}

	
auto opaque_multi_ndarray_format::extract_part(std::ptrdiff_t index) const -> extracted_part {
	const auto& prt = part_at(index);	
	return extracted_part {
		std::make_shared<opaque_ndarray_format>(prt.format),
		prt.offset
	};
}


void opaque_multi_ndarray_format::copy_frame(frame_ptr destination, const_frame_ptr origin) const {
	if(is_contiguous_pod()) {
		std::memcpy(destination, origin, frame_size());
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < parts_count(); ++part_index) {
			const auto& pt = part_at(part_index);			
			ndarray_data_copy(
				advance_raw_ptr(destination, pt.offset),
				advance_raw_ptr(origin, pt.offset),
				pt.format
			);
		}
	}
}


bool opaque_multi_ndarray_format::compare_frame(const_frame_ptr a, const_frame_ptr b) const {
	if(is_contiguous_pod()) {
		return (std::memcmp(a, b, frame_size()) == 0);
	} else {
		for(std::ptrdiff_t part_index = 0; part_index < parts_count(); ++part_index) {
			const auto& pt = part_at(part_index);
			bool part_equal = ndarray_data_compare(
				advance_raw_ptr(a, pt.offset),
				advance_raw_ptr(b, pt.offset),
				pt.format
			);
			if(!part_equal) return false;
		}
		return true;
	}
}

}
