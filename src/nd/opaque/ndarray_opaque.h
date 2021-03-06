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

#ifndef MF_NDARRAY_OPAQUE_H_
#define MF_NDARRAY_OPAQUE_H_

#include "ndarray_view_opaque.h"
#include "../detail/ndarray_wrapper.h"
#include "../../os/memory.h"


namespace mf {

/// Container for \ref ndarray_view_opaque.
template<std::size_t Dim, typename Allocator = raw_allocator>
class ndarray_opaque :
public detail::ndarray_wrapper<ndarray_view_opaque<Dim, true>, ndarray_view_opaque<Dim, false>, Allocator> {
	using base = detail::ndarray_wrapper<ndarray_view_opaque<Dim, true>, ndarray_view_opaque<Dim, false>, Allocator>;
	
public:
	using typename base::view_type;
	using typename base::const_view_type;
	using typename base::shape_type;
	using typename base::strides_type;
	
	/// \name Constructor
	///@{
	/// Construct null \ref ndarray_opaque.
	explicit ndarray_opaque(const Allocator& = Allocator());
	
	/// Construct empty \ref ndarray_opaque with given shape and frame format.
	/** Has default strides, optionally with specified frame padding. */
	ndarray_opaque
	(const shape_type& shape, const ndarray_opaque_frame_format& frm, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Construct \ref ndarray_opaque with shape and copy of elements from a \ref ndarray_view_opaque.
	/** Has default strides, optionally with specified frame padding. Does not take strides from \a vw. */
	explicit ndarray_opaque(const const_view_type& vw, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Copy-construct from another \ref ndarray_opaque of same type.
	/** Takes strides from \a arr. */
	ndarray_opaque(const ndarray_opaque& arr);
	
	/// Move-construct from another \ref ndarray_opaque of same type.
	/** Takes strides from \a arr and sets \a arr to null. */
	ndarray_opaque(ndarray_opaque&& arr);
	///@}
	

	/// \name Deep assignment
	///@{
	/// Assign shape and elements from \ref vw.
	/** Resets to default strides, optionally with specified frame padding. Reallocates memory if necessary. */
	void assign(const const_view_type& vw, std::size_t frame_padding = 0);
	
	/// Assign shape and elements from \ref vw.
	/** Equivalent to `assign(vw)`. */
	ndarray_opaque& operator=(const const_view_type& vw)
		{ assign(vw); return *this; }

	/// Copy-assign from another \ref ndarray_opaque.
	/** Takes strides from \a arr */
	ndarray_opaque& operator=(const ndarray_opaque& arr);

	/// Move-assign from another \ref ndarray_opaque.
	/** Takes strides from \a arr and sets \a arr to null. */
	ndarray_opaque& operator=(ndarray_opaque&& arr);
	///@}
	
	
	/// \name Attributes
	///@{
	const ndarray_opaque_frame_format& frame_format() const noexcept { return base::get_view_().format(); }
	[[deprecated]] const ndarray_opaque_frame_format& format() const noexcept { return base::get_view_().format(); }
	///@}
};


template<std::size_t Dim>
ndarray_view_opaque<Dim, true> extract_part(ndarray_opaque<Dim>& arr, std::ptrdiff_t part_index) {
	return extract_part(arr.view(), part_index);
}


template<std::size_t Dim>
ndarray_view_opaque<Dim, false> extract_part(const ndarray_opaque<Dim>& arr, std::ptrdiff_t part_index) {
	return extract_part(arr.cview(), part_index);
}


}

#include "ndarray_opaque.tcc"

#endif
