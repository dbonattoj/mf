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

#ifndef MF_NDARRAY_OPAQUE_FORMAT_MULTI_ARRAY_H_
#define MF_NDARRAY_OPAQUE_FORMAT_MULTI_ARRAY_H_

#include "../ndarray_format.h"
#include <vector>

namespace mf {

/// Format of ndarray opaque frame composed of one or multiple arrays with different \ref ndarray_format.
/** Frame consists of multiple arrays after each other. Necessary padding is inserted between arrays and at the end of
 ** the frames, to satisfy alignment requirement of the parts. */
class opaque_format_multi_array {
public:
	using frame_ptr = void*;
	using const_frame_ptr = const void*;

	struct part {
		std::ptrdiff_t offset;
		ndarray_format format;
	};
	
	friend bool operator==(const part& a, const part& b) { return (a.offset == b.offset) && (a.format == b.format); }
	friend bool operator!=(const part& a, const part& b) { return (a.offset != b.offset) || (a.format != b.format); }
	
private:
	std::vector<part> parts_;
	std::size_t frame_size_without_end_padding_ = 0;
	std::size_t frame_alignment_requirement_ = 1;
	
	std::size_t frame_size_with_end_padding_ = 0;
	bool contiguous_ = false;
	
	void update_frame_size_with_end_padding_();

public:
	opaque_format_multi_array();
	
	opaque_format_multi_array(const opaque_format_multi_array&) = default;
	opaque_format_multi_array& operator=(const opaque_format_multi_array&) = default;
	
	std::size_t parts_count() const { Assert(is_defined()); return parts_.size(); }
	const part& part_at(std::ptrdiff_t part_index) const { Assert(is_defined()); return parts_.at(part_index); }
	const part& add_part(const ndarray_format& format);
	
	bool is_contiguous() const { Assert(is_defined()); return contiguous_; }

	bool is_defined() const noexcept { return (frame_size_without_end_padding_ > 0); }
	std::size_t frame_size() const noexcept { return frame_size_with_end_padding_; }
	std::size_t frame_alignment_requirement() const noexcept { return frame_alignment_requirement_; }
	bool is_contiguous_pod() { return is_contiguous(); }

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const;
	void construct_frame(frame_ptr) const;
	void destruct_frame(frame_ptr) const;
	
	friend bool operator==(const opaque_format_multi_array&, const opaque_format_multi_array&);
	friend bool operator!=(const opaque_format_multi_array&, const opaque_format_multi_array&);
};


}

#endif

