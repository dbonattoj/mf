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

#include "../../common.h"
#include "../../utility/misc.h"
#include <typeinfo>
#include <typeindex>

namespace mf {

/// Format information of type-erased frame of `ndarray_view_generic`.
/** Assumes the data is an array of an unknown element type. Stores element size and alignment requirement in bytes.
 ** Also stores the stride (in bytes) of the elements in the actual data. */
class frame_format {
private:
	std::size_t elem_size_ = 0; ///< sizeof of element.
	std::size_t elem_alignment_ = 0; ///< alignof of elememt.
	
	/// Stride of elements in frame.
	/** Must be greater or equal to `elem_size_`, and multiple of `elem_alignment_`. */
	std::size_t stride_ = 0;
	
public:
	frame_format() = default;
	
	frame_format(const frame_format&) = default;
	frame_format& operator=(const frame_format&) = default;
	
	template<typename Elem>
	static frame_format default_format(std::size_t stride = sizeof(Elem)) {
		static_assert(sizeof(Elem) >= alignof(Elem), "sizeof always larger or equal to alignof");
		Expects(stride >= sizeof(Elem) && is_nonzero_multiple_of(stride, alignof(Elem)));
		frame_format format;
		format.elem_size_ = sizeof(Elem);
		format.elem_alignment_ = alignof(Elem);
		format.stride_ = stride;
		return format;
	}
	
	std::size_t elem_size() const noexcept { return elem_size_; }
	std::size_t elem_alignment() const noexcept { return elem_alignment_; }
	std::size_t stride() const noexcept { return stride_; }
	std::size_t padding() const noexcept { return stride_ - elem_size_; }
};


}

#endif

