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

#ifndef MF_NDARRAY_OPAQUE_FORMAT_RAW_H_
#define MF_NDARRAY_OPAQUE_FORMAT_RAW_H_

#include <cstring>

namespace mf {

/// Format of ndarray opaque frame the consists raw data with a given size and alignment.
class opaque_format_raw {
public:
	using frame_ptr = void*;
	using const_frame_ptr = const void*;

private:
	std::size_t frame_size_ = 0;
	std::size_t frame_alignment_requirement_ = 1;

public:
	opaque_format_raw() = default;
	explicit opaque_format_raw(std::size_t frame_size, std::size_t frame_alignment_requirement = 1) :
		frame_size_(frame_size), frame_alignment_requirement_(frame_alignment_requirement) { }
	
	opaque_format_raw(const opaque_format_raw&) = default;
	opaque_format_raw& operator=(const opaque_format_raw&) = default;

	bool is_defined() const noexcept { return (frame_size_ > 0); }
	std::size_t frame_size() const noexcept { return frame_size_; }
	std::size_t frame_alignment_requirement() const noexcept { return frame_alignment_requirement_; }
	bool is_contiguous_pod() { return true; }
	
	void copy_frame(frame_ptr destination, const_frame_ptr origin) const {
		std::memcpy(destination, origin, frame_size());
	}
	
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const {
		return (std::memcmp(a, b, frame_size()) == 0);
	}
	
	void construct_frame(frame_ptr) const { }
	void destruct_frame(frame_ptr) const { }
};



inline bool operator==(const ndarray_opaque_format_raw& a, const ndarray_opaque_format_raw& b) {
	return (a.frame_size() == b.frame_size()) && (a.frame_alignment_requirement() == b.frame_alignment_requirement());
}

inline bool operator!=(const ndarray_opaque_format_raw& a, const ndarray_opaque_format_raw& b) {
	return (a.frame_size() != b.frame_size()) || (a.frame_alignment_requirement() != b.frame_alignment_requirement());
}



}

#endif

