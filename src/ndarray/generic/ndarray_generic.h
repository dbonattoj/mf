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

#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "frame_format.h"
#include "ndarray_view_generic.h"
#include "../ndarray.h"
#include "../../elem.h"
#include "../../os/memory.h"

namespace mf {

/// Properties for construction of \ref ndarray_generic and derived container objects.
/** Described frame format, frame length and number of frames. Does not describe padding between array frames, because
 ** this may be determined internally by the container. (For example, \ref ring needs to adjust is to fit page size.) */
struct ndarray_generic_properties {
	frame_format format_; ///< Element type and alignment of frame.
	std::size_t frame_length_; ///< Number of elements in one frame.
	std::size_t array_length_; ///< Number of frames.
	
	const frame_format& format() const noexcept { return format_; }
	std::size_t frame_length() const noexcept { return frame_length_; }
	std::size_t array_length() const noexcept { return array_length_; }
	std::size_t frame_size() const noexcept { return frame_length_ * format_.stride(); }

	ndarray_generic_properties(const frame_format& format, std::size_t frame_length, std::size_t array_length) :
		format_(format), frame_length_(frame_length), array_length_(array_length) { }
};


/// Array container with type erased multidimensional frames.
/** Like \ref ndarray_view_generic, it retains \ref frame_format with element type and alignment of frames.  */
template<typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<2, byte, Allocator> {
	using base = ndarray<2, byte, Allocator>;

private:
	frame_format format_;

public:
	using view_type = ndarray_view_generic<2>;

	ndarray_generic(const ndarray_generic_properties&, std::size_t padding = 0, const Allocator& = Allocator());
	explicit ndarray_generic(const ndarray_generic&) = default;

	const frame_format& format() const noexcept { return format_; }

	view_type view() { return view_type(base::view(), format_); }	
};


}

#include "ndarray_generic.tcc"

#endif
