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

namespace mf {	
	
ndarray_opaque_frame_format::ndarray_opaque_frame_format(const ndarray_format& format) {
	parts_.push_back({ 0, format });
	frame_size_ = format.frame_size();
	frame_alignment_requirement_ = format.frame_alignment_requirement();
}

auto ndarray_opaque_frame_format::add_part(const ndarray_format& new_part_format) -> const part& {
	part new_part { 0, new_part_format };
	
	if(parts_.size() > 0) {
		const part& previous_part = parts_.back();
		std::ptrdiff_t min_offset = previous_part.offset;
		std::size_t alignment_requirement = new_part_format.frame_alignment_requirement();
		
		if(is_multiple_of(min_offset, alignment_requirement)) new_part.offset = min_offset;
		else new_part.offset = (1 + (min_offset / alignment_requirement)) * alignment_requirement;

		Assert(is_multiple_of(offset, alignment_requirement));
		Assert(offset >= min_offset);
	}
	
	parts_.push_back(new_part);
	frame_size_ = new_part.offset + new_part.format.frame_size();
	frame_alignment_requirement_ = lcm(frame_alignment_requirement_, new_part.format.frame_alignment_requirement());
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


}
