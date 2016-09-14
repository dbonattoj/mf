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

#ifndef MF_NDARRAY_OPAQUE_NDARRAY_FORMAT_H_
#define MF_NDARRAY_OPAQUE_NDARRAY_FORMAT_H_

#include "opaque_format.h"
#include "../ndarray_format.h"

namespace mf {

/// Format of \ref ndarray_view_opaque frame containing \ref ndarray_format data.
/** The \ref ndarray_view_opaque can be casted from and to an \ref ndarray_view having this format. */
class opaque_ndarray_format : public opaque_format {
private:
	ndarray_format array_format_;

public:
	explicit opaque_ndarray_format(const ndarray_format& form);
	opaque_ndarray_format(const opaque_ndarray_format&) = default;
	opaque_ndarray_format(opaque_ndarray_format&&) = default;
	
	opaque_ndarray_format& operator=(const opaque_ndarray_format&) = default;
	opaque_ndarray_format& operator=(opaque_ndarray_format&&) = default;
	
	bool compare(const opaque_format&) const override;

	void copy_frame(frame_ptr destination, const_frame_ptr origin) const override;
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const override;
	void construct_frame(frame_ptr) const override { }
	void destruct_frame(frame_ptr) const override { }
	void initialize_frame(frame_ptr) const override { }
	
	bool has_array_format() const override { return true; }
	ndarray_format array_format() const override { return array_format_; }
	std::ptrdiff_t array_offset() const override { return 0; }
};


inline bool operator==(const opaque_ndarray_format& a, const opaque_ndarray_format& b) {
	return (a.array_format() == b.array_format());
}

inline bool operator!=(const opaque_ndarray_format& a, const opaque_ndarray_format& b) {
	return (a.array_format() != b.array_format());
}

}

#endif
