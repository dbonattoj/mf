#include "frame_format.h"
#include "../../utility/misc.h"

namespace mf { namespace flow {

array_frame_format::array_frame_format
(const elem_type_info& elem_type, std::size_t elem_count, std::size_t elem_stride, std::size_t offset) :
	elem_type_(elem_type),
	elem_count_(elem_count),
	elem_stride_(elem_stride),
	offset_(offset)
{
	Expects(elem_count > 0);
	Expects(is_multiple_of(offset, elem_type.alignment));
	Expects(is_multiple_of(elem_stride, elem_type.alignment));
	Expects(elem_stride >= elem_type.size);
}


void composite_frame_format::add_nested_frame(const array_frame_format& nested) {
	nested_frames_.push_back(nested);
	total_length_ = std::max(total_length_, nested.frame_length());
	total_aligmnent_requirement_ = lcm(total_aligmnent_requirement_, nested.frame_alignment_requirement());
}


const array_frame_format& composite_frame_format::place_next_nested_frame(const array_frame_format& nested) {
	Expects(nested.offset() == 0);
	
	if(nested_frames_.size() > 0) {
		const composite_frame_format& previous = nested_frames_.back();
		
		std::size_t min_offset = previous.frame_length();
		std::size_t align_requirement = nested.frame_alignment_requirement();
		std::size_t offset;
		if(is_multiple_of(min_offset, align_requirement)) offset = min_offset;
		else offset = (1 + (min_offset / align_requirement)) * alignment_requirement;
		Assert(is_multiple_of(offset, align_requirement));
		Assert(offset >= min_offset);
		
		add_nested_frame(array_frame_format(nested.elem_type(), nested.elem_count(), nested.elem_stride(), offset));
		
	} else {
		add_nested_frame(nested);
	}
	
	return nested_frames_.back();
}


}}
