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

#include <utility>

namespace mf {

template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(const Allocator& allocator) :
	base(allocator) { }


template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque
(const shape_type& shape, const frame_format& frm, std::size_t frame_padding, const Allocator& allocator) :
base(
	shape,
	view_type::default_strides(frm, shape, frame_padding),
	(frm.frame_size() + frame_padding) * shape.product(),
	frm.frame_alignment_requirement(),
	allocator,
	frm
) { }

	
template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque
(const const_view_type& vw, std::size_t frame_padding, const Allocator& allocator) :
base(
	vw.shape(),
	view_type::default_strides(vw.shape(), frame_padding),
	(vw.format().frame_size() + frame_padding) * vw.shape().product(),
	vw.format().frame_alignment_requirement(),
	allocator,
	vw.format()
) {
	base::view().assign(vw);
}

	
template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(const ndarray_opaque& arr) :
base(
	arr.shape(),
	arr.strides(),
	arr.allocated_byte_size(),
	arr.format().format_alignment_requirement(),
	arr.get_allocator(),
	arr.format()
) {
	base::view().assign(arr.cview());
}
	

template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(ndarray_opaque&& arr) :
base(std::move(arr)) { }
	

template<std::size_t Dim, typename Allocator>
void ndarray_opaque<Dim, Allocator>::assign(const const_view_type& vw, std::size_t frame_padding) {
	base::reset_(
		vw.shape(),
		view_type::default_strides(vw.shape(), frame_padding),
		(vw.format().frame_size() + frame_padding) * vw.shape().product(),
		vw.format().frame_alignment_requirement(),
		vw.format()
	);
	base::view().assign(vw);
}
	

template<std::size_t Dim, typename Allocator>
auto ndarray_opaque<Dim, Allocator>::operator=(const ndarray_opaque& arr) -> ndarray_opaque& {
	if(&arr == this) return *this;
	base::reset_(
		arr.shape(),
		arr.strides(),
		arr.allocated_size(),
		arr.format().frame_alignment_requirement()
	);
	base::view().assign(arr.cview());
	return *this;
}


template<std::size_t Dim, typename Allocator>
auto ndarray_opaque<Dim, Allocator>::operator=(ndarray_opaque&& arr) -> ndarray_opaque& {
	base::operator=(std::move(arr));
	return *this;
}


}
