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

#ifndef MF_NDARRAY_FRAME_FORMAT_H_
#define MF_NDARRAY_FRAME_FORMAT_H_

#include <cstdlib>
#include "../utility/misc.h"

namespace mf {

/// Runtime description of data format in \ref ndarray.
/** Defines byte size and alignment requirement of element type (aka `sizeof(Elem)`, `alignof(Elem)`),
 ** number of elements in array (corresponds to `ndarray::size()`), and stride between elements in bytes.
 ** Describes data in a \ref ndarray_view, with default (possibly padded) strides.
 ** Does not define the dimensionality, shape components, and elem type. */
class ndarray_format {
private:		
	std::size_t elem_size_ = 0;
	std::size_t elem_alignment_ = 0;

	std::size_t length_ = 0;
	std::size_t stride_ = 0;

public:
	ndarray_format() = default;
	ndarray_format(std::size_t elem_size, std::size_t elem_alignment, std::size_t length, std::size_t stride) :
		elem_size_(elem_size), elem_alignment_(elem_alignment), length_(length), stride_(stride)
	{
		Assert(is_nonzero_multiple_of(stride, elem_alignment));
		Assert(stride >= elem_size);
	}
	
	ndarray_format(const ndarray_format&) = default;
	ndarray_format& operator=(const ndarray_format&) = default;

	bool is_defined() const { return (elem_size_ != 0); }

	std::size_t frame_size() const noexcept { return length_ * stride_; }
	std::size_t frame_alignment_requirement() const noexcept { return elem_alignment_; }

	std::size_t length() const noexcept { return length_; }
	std::size_t stride() const noexcept { return stride_; }
	
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t elem_alignment() const noexcept { return elem_alignment_; }

	std::size_t elem_padding() const noexcept { return stride() - elem_size(); }
	
	bool is_contiguous() const noexcept { return (elem_padding() == 0); }
};


/// Compare two data stored in \a a and \a b, both having format \a frame_format.
bool ndarray_frame_compare(const void* a, const void* b, const ndarray_format& frame_format);

/// Copy data at \a origin having format \a frame_format to \a destination.
void ndarray_frame_copy(void* destination, const void* origin, const ndarray_format& frame_format);


bool operator==(const ndarray_format&, const ndarray_format&);
bool operator!=(const ndarray_format&, const ndarray_format&);


template<typename Elem>
ndarray_format make_ndarray_format(std::size_t length, std::size_t stride = sizeof(Elem)) {
	return ndarray_format(sizeof(Elem), alignof(Elem), length, stride);
}


}

#endif

