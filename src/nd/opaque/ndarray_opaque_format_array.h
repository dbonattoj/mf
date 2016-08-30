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

#ifndef MF_NDARRAY_OPAQUE_FORMAT_ARRAY_H_
#define MF_NDARRAY_OPAQUE_FORMAT_ARRAY_H_

#include "../ndarray_format.h"
#include "ndarray_view_opaque_fwd.h"

namespace mf {

/// Format of ndarray opaque frame the consists of an array define by \ref ndarray_format.
/** \ref ndarray_opaque_view with this format can be casted to and from \ref ndarray_view whose shape, dimension, 
 ** stride and element type agrees with the \ref ndarray_format. */
class ndarray_opaque_format_array {
public:
	using frame_ptr = void*;
	using const_frame_ptr = const void*;
	
private:
	ndarray_format array_format_;

public:
	ndarray_opaque_format_array() : array_format_() { }
	explicit ndarray_opaque_format_array(const ndarray_format& frm) : array_format_(frm) { }
	
	ndarray_opaque_format_array(const ndarray_opaque_format_array&) = default;
	ndarray_opaque_format_array& operator=(const ndarray_opaque_format_array&) = default;	
		
	const ndarray_format& array_format() const { return array_format_; }
	ndarray_format& array_format() { return array_format_; }
	bool is_contiguous() const { Assert(is_defined()); return array_format_.is_contiguous(); }	
	
	bool is_defined() const noexcept { return array_format_.is_defined(); }
	std::size_t frame_size() const noexcept { return array_format_.frame_size(); }
	std::size_t frame_alignment_requirement() const noexcept { return array_format_.frame_alignment_requirement(); }
	bool is_contiguous_pod() { return is_contiguous(); }
	
	void copy_frame(frame_ptr destination, const_frame_ptr origin) const {
		ndarray_data_copy(destination, origin, array_format_);
	}
	
	bool compare_frame(const_frame_ptr a, const_frame_ptr b) const {
		return ndarray_data_compare(a, b, array_format_);
	}
	
	void construct_frame(frame_ptr) const { }
	void destruct_frame(frame_ptr) const { }
};


inline bool operator==(const ndarray_opaque_format_array& a, const ndarray_opaque_format_array& b) {
	return a.array_format() == b.array_format();
}


inline bool operator!=(const ndarray_opaque_format_array& a, const ndarray_opaque_format_array& b) {
	return a.array_format() != b.array_format();
}


/// Cast input \ref ndarray_view to opaque \ref ndarray_view_opaque with given dimension.
template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_view<Concrete_dim, Concrete_elem>& concrete_view);



/// Cast input \ref ndarray_view_opaque to concrete \ref ndarray_view with given dimension, frame shape and element type.
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_view_opaque<Opaque_dim, ndarray_opaque_format_array, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
);


}

#include "ndarray_opaque_format_array.tcc"

#endif

