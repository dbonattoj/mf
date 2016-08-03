#include "frame_format.h"
#include "../../utility/misc.h"

namespace mf { namespace flow {

frame_format::frame_format(const frame_array_format& arr_format) {
	add_array_frame_(arr_format);
}


frame_format& frame_format::operator=(const frame_array_format& arr_format) {
	arrays_.clear();
	total_size_ = 0;
	total_aligmnent_requirement_ = 1;
	add_array_frame_(arr_format);
	arrays_.shrink_to_fit();
	return *this;
}


void frame_format::add_array_frame_(const frame_array_format& arr_format) {
	Expects(arr_format.is_defined());
	arrays_.push_back(arr_format);
	total_size_ = std::max(total_size_, arr_format.frame_size());
	total_aligmnent_requirement_ = lcm(total_aligmnent_requirement_, arr_format.frame_alignment_requirement());
}


const array_frame_format& frame_format::place_next_array_frame(const frame_array_format& nested) {
	Expects(nested.offset() == 0);
	
	if(arrays_.size() > 0) {
		const composite_frame_format& previous = arrays_.back();
		
		std::size_t min_offset = previous.frame_size();
		std::size_t align_requirement = nested.frame_alignment_requirement();
		
		std::size_t offset;
		if(is_multiple_of(min_offset, align_requirement)) offset = min_offset;
		else offset = (1 + (min_offset / align_requirement)) * alignment_requirement;
		
		Assert(is_multiple_of(offset, align_requirement));
		Assert(offset >= min_offset);
		
		add_array_frame_(array_frame_format(nested.elem_type(), nested.elem_count(), nested.elem_stride(), offset));
		
	} else {
		add_array_frame_(nested);
	}
	
	return arrays_.back();
}


}}
