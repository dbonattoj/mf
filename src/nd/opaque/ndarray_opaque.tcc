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
void ndarray_opaque<Dim, Allocator>::construct_frames_() {
	if(frame_format().is_pod()) return;
	
}


template<std::size_t Dim, typename Allocator>
void ndarray_opaque<Dim, Allocator>::destruct_frames_() {
	if(frame_format().is_pod()) return;
	
}


template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(const Allocator& allocator) :
	base(allocator) { }


template<std::size_t Dim, typename Allocator> template<typename Format>
ndarray_opaque<Dim, Allocator>::ndarray_opaque
(const shape_type& shape, Format&& frm, std::size_t frame_padding, const Allocator& allocator) :
	ndarray_opaque(shape, forward_make_shared(frm), frame_padding, allocator) { }


template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque
(const shape_type& shape, const format_ptr& frm, std::size_t frame_padding, const Allocator& allocator) :
base(
	shape,
	view_type::default_strides(shape, *frm, frame_padding),
	(frm->frame_size() + frame_padding) * shape.product(),
	frm->frame_alignment_requirement(),
	allocator,
	frm
) {
	construct_frames_();
}

	
template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque
(const const_view_type& vw, std::size_t frame_padding, const Allocator& allocator) :
	base(allocator)
{
	if(vw.is_null()) return;
	base::reset(
		vw.shape(),
		view_type::default_strides(vw.shape(), vw.frame_format(), frame_padding),
		(vw.frame_format().frame_size() + frame_padding) * vw.shape().product(),
		vw.format().frame_alignment_requirement(),
		vw.frame_format_ptr()
	);
	construct_frames_();
	base::view().assign(vw);
}

	
template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(const ndarray_opaque& arr) :
	base(arr.get_allocator())
{
	if(arr.is_null()) return;
	base::reset(
		arr.shape(),
		arr.strides(),
		arr.allocated_byte_size(),
		arr.frame_format().frame_alignment_requirement(),
		arr.frame_format()
	);
	construct_frames_();
	base::view().assign(arr.cview());
}
	

template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::ndarray_opaque(ndarray_opaque&& arr) :
	base(std::move(arr)) { }


template<std::size_t Dim, typename Allocator>
ndarray_opaque<Dim, Allocator>::~ndarray_opaque() {
	destruct_frames_();
}
	

template<std::size_t Dim, typename Allocator>
void ndarray_opaque<Dim, Allocator>::assign(const const_view_type& vw, std::size_t frame_padding) {
	destruct_frames_();
	if(vw.is_null()) {
		base::reset_();
	} else {
		base::reset_(
			vw.shape(),
			view_type::default_strides(vw.shape(), vw.format(), frame_padding),
			(vw.format().frame_size() + frame_padding) * vw.shape().product(),
			vw.format().frame_alignment_requirement(),
			vw.format()
		);
		construct_frames_();
		base::view().assign(vw);
	}
}
	

template<std::size_t Dim, typename Allocator>
auto ndarray_opaque<Dim, Allocator>::operator=(const ndarray_opaque& arr) -> ndarray_opaque& {
	if(&arr == this) return *this;
	destruct_frames_();
	if(arr.is_null()) {
		base::reset_();
	} else {
		base::reset_(
			arr.shape(),
			arr.strides(),
			arr.allocated_size(),
			arr.format().frame_alignment_requirement(),
			arr.format()
		);
		construct_frames_();
		base::view().assign(arr.cview());
	}
	return *this;
}


template<std::size_t Dim, typename Allocator>
auto ndarray_opaque<Dim, Allocator>::operator=(ndarray_opaque&& arr) -> ndarray_opaque& {
	base::operator=(std::move(arr));
	return *this;
}


}
