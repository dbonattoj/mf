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

#include "frame_format.h"
#include "../../utility/misc.h"

namespace mf {

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


const frame_array_format& frame_format::place_next_array(const frame_array_format& nested) {
	Expects(nested.offset() == 0);
	
	if(arrays_.size() > 0) {
		const frame_array_format& previous = arrays_.back();
		
		std::size_t min_offset = previous.frame_size();
		std::size_t alignment_requirement = nested.frame_alignment_requirement();
		
		std::size_t offset;
		if(is_multiple_of(min_offset, alignment_requirement)) offset = min_offset;
		else offset = (1 + (min_offset / alignment_requirement)) * alignment_requirement;
		
		Assert(is_multiple_of(offset, alignment_requirement));
		Assert(offset >= min_offset);
		
		frame_array_format new_nested = nested;
		new_nested.set_offset(offset);
		add_array_frame_(new_nested);
		
	} else {
		add_array_frame_(nested);
	}
	
	return arrays_.back();
}


}
