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
#include "../../utility/misc.h"


namespace mf {

/// Container for \ref ndarray_view_opaque.
template<std::size_t Dim, typename Allocator = raw_allocator>
class ndarray_opaque :
public detail::ndarray_wrapper<ndarray_view_opaque<Dim, true>, ndarray_view_opaque<Dim, false>, Allocator> {
	using base = detail::ndarray_wrapper<ndarray_view_opaque<Dim, true>, ndarray_view_opaque<Dim, false>, Allocator>;

private:
	void construct_frames_();
	void destruct_frames_();

public:
	using typename base::view_type;
	using typename base::const_view_type;
	using typename base::shape_type;
	using typename base::strides_type;
	using format_base_type = typename view_type::format_base_type;
	using format_ptr = typename view_type::format_ptr;

	using frame_ptr = typename view_type::frame_ptr;
	using const_frame_ptr = typename const_view_type::frame_ptr;
	
	/// \name Constructor
	///@{	
	/// Construct empty \ref ndarray_opaque with given shape and frame format.
	/** Gets default strides, optionally with specified frame padding. Memory is allocated, and frames are *constructed*
	 ** is required by the format. */
	template<typename Format, typename = enable_if_derived_from_opaque_format<Format>>
	ndarray_opaque(const shape_type&, Format&&, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Construct empty \ref ndarray_opaque with given shape and frame format.
	/** Gets default strides, optionally with specified frame padding. Memory is allocated, and frames are *constructed*
	 ** is required by the format. */
	ndarray_opaque(const shape_type&, const format_ptr&, std::size_t frame_padding = 0, const Allocator& = Allocator());

	/// Construct \ref ndarray_opaque with shape and format, and copy of elements from a \ref ndarray_view_opaque.
	/** Gets default strides, optionally with specified frame padding. Does not take strides from \a vw.
	 ** Allocates memory, and *constructs* frames is required by the format. Then copies the frames. */
	explicit ndarray_opaque(const const_view_type& vw, std::size_t frame_padding = 0, const Allocator& = Allocator());
	
	/// Copy-construct from another \ref ndarray_opaque of same type.
	/** Takes strides from \a arr.
	 ** Allocates memory, and *constructs* frames is required by the format. Then copies the frames. */
	ndarray_opaque(const ndarray_opaque&);
	
	/// Move-construct from another \ref ndarray_opaque of same type.
	/** Takes strides from \a arr and sets \a arr to null. Does not allocate, deallocate or copy memory. */
	ndarray_opaque(ndarray_opaque&&);
	
	/// Destruct the \ref ndarray_opaque.
	/** Deallocates memory, and *destructs* frames is required by the format. */
	~ndarray_opaque();
	///@}
	

	/// \name Deep assignment
	///@{
	/// Assign shape and elements from \ref vw.
	/** Resets to default strides, optionally with specified frame padding. Reallocates memory if necessary.
	 ** \a vw must have same frame format as this. 
	 ** If required by format, *destructs* old frames and, and *constructs* new frames before copying. */
	void assign(const const_view_type& vw, std::size_t frame_padding = 0);
	
	/// Assign shape and elements from \ref vw.
	/** Equivalent to `assign(vw)`. */
	ndarray_opaque& operator=(const const_view_type& vw)
		{ assign(vw); return *this; }

	/// Copy-assign from another \ref ndarray_opaque.
	/** Takes strides and format from \a arr. Reallocates memory if necessary.
	 ** If required by formats, *destructs* old frames and, and *constructs* new frames before copying. */
	ndarray_opaque& operator=(const ndarray_opaque& arr);

	/// Move-assign from another \ref ndarray_opaque.
	/** Takes strides and format from \a arr and sets \a arr to null. Does not allocate, deallocate or copy memory. */
	ndarray_opaque& operator=(ndarray_opaque&& arr);
	///@}
	
	
	/// \name Attributes
	///@{
	const format_ptr& frame_format_ptr() const { return base::get_view_().frame_format_ptr(); }
	const format_base_type& frame_format() const { return base::get_view_().frame_format(); }
	///@}
	
	
	void initialize_frame(frame_ptr);
};


template<std::size_t Dim, typename Allocator>
auto extract_part(const ndarray_opaque<Dim, Allocator>& vw, std::ptrdiff_t part_index) {
	return extract_part(vw.cview(), part_index);
}


template<std::size_t Dim, typename Allocator>
auto extract_part(ndarray_opaque<Dim, Allocator>& vw, std::ptrdiff_t part_index) {
	return extract_part(vw.view(), part_index);
}




}

#include "ndarray_opaque.tcc"

#endif
