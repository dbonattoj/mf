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


void opaque_multi_ndarray_format::update_frame_size_with_end_padding_() {
	if(is_multiple_of(frame_size_without_end_padding_, frame_alignment_requirement_)) {
		frame_size_with_end_padding_ = frame_size_without_end_padding_;
	} else {
		std::size_t remainder = frame_size_without_end_padding_ % frame_alignment_requirement_;
		frame_size_with_end_padding_ = frame_size_without_end_padding_ + (frame_alignment_requirement_ - remainder);
	}
	Assert(is_multiple_of(frame_size_with_end_padding_, frame_alignment_requirement_));
}


auto opaque_multi_ndarray_format::add_part(const ndarray_format& new_part_format) -> const part& {
	part new_part { new_part_format, 0 };
	
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
