#include "opaque_multi_ndarray_format.h"

namespace mf {
	
bool opaque_multi_ndarray_format::compare(const opaque_format& frm) {
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

	
auto opaque_multi_ndarray_format::extract_part(std::ptrdiff_t index) const override -> extracted_part {
	const auto& prt = part_at(index);	
	return extracted_part {
		std::make_shared<opaque_ndarray_format>(prt.format),
		prt.offset
	};
}

}
