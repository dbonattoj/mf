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

#ifndef MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_
#define MF_NDARRAY_OPAQUE_FRAME_FORMAT_H_

#include "../ndarray_format.h"
#include <vector>

namespace mf {

class ndarray_opaque_frame_format {
public:
	struct part {
		std::ptrdiff_t offset;
		ndarray_format format;
	};
	
	friend bool operator==(const part& a, const part& b) { return (a.offset == b.offset) && (a.format == b.format); }
	friend bool operator!=(const part& a, const part& b) { return (a.offset != b.offset) || (a.format != b.format); }
	
private:
	std::vector<part> parts_;
	std::size_t frame_size_ = 0;
	std::size_t frame_alignment_requirement_ = 1;
	
	std::size_t frame_size_with_padding_ = 0;
	bool contiguous_ = false;
	
	void update_frame_size_with_padding_();

public:
	ndarray_opaque_frame_format();
	explicit ndarray_opaque_frame_format(std::size_t frame_size, std::size_t frame_alignment_requirement = 1);
	explicit ndarray_opaque_frame_format(const ndarray_format&);
	
	ndarray_opaque_frame_format(const ndarray_opaque_frame_format&) = default;
	ndarray_opaque_frame_format& operator=(const ndarray_opaque_frame_format&) = default;
	
	const part& add_part(const ndarray_format& format);
	
	std::size_t frame_size() const noexcept { return frame_size_; }
	std::size_t frame_size_with_padding() const noexcept { return frame_size_with_padding_; }
	std::size_t frame_alignment_requirement() const noexcept { return frame_alignment_requirement_; }
	
	bool is_defined() const noexcept { return (frame_size_ > 0); }
	bool is_raw() const noexcept { return is_defined() && (parts_.size() == 0); }
		
	std::size_t parts_count() const { Assert(is_defined()); return parts_.size(); }
	bool is_multi_part() const { return (parts_count() > 1); }
	const part& part_at(std::ptrdiff_t part_index) const { Assert(is_defined()); return parts_.at(part_index); }

	bool is_single_part() const { return (parts_count() == 1); }
	const ndarray_format& array_format() const { Assert_crit(is_single_part()); return parts_.front().format; }
	
	bool is_contiguous() const { Assert(is_defined()); return contiguous_; }
	
	friend bool operator==(const ndarray_opaque_frame_format&, const ndarray_opaque_frame_format&);
	friend bool operator!=(const ndarray_opaque_frame_format&, const ndarray_opaque_frame_format&);
};


/// Compare two data stored in \a a and \a b, both having format \a frame_format.
bool ndarray_opaque_frame_compare(const void* a, const void* b, const ndarray_opaque_frame_format& frame_format);

/// Copy data at \a origin having format \a frame_format to \a destination.
void ndarray_opaque_frame_copy(void* destination, const void* origin, const ndarray_opaque_frame_format& frame_format);


}

#endif

