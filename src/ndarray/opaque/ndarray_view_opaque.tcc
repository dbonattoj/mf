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


namespace mf {


template<std::size_t Dim, bool Mutable>
ndarray_view_opaque<Dim, Mutable>::ndarray_view_opaque
(frame_ptr start, const shape_type& shape, const strides_type& strides, const ndarray_opaque_frame_format& frm) :
	base(static_cast<base_value_type*>(start), ndcoord_cat(shape, frm.frame_size()), ndcoord_cat(strides, 1)),
	format_(frm) { }


template<std::size_t Dim, bool Mutable>
ndarray_view_opaque<Dim, Mutable> extract_part
(const ndarray_view_opaque<Dim, Mutable>& vw, std::ptrdiff_t part_index) {
	const auto& format_part = vw.format().part_index(part_index);
	auto new_start = advance_raw_ptr(vw.start(), format_part.offset);
	return ndarray_view_opaque<Dim, Mutable>(new_start, vw.shape(), vw.strides(), format_part.format);
}


//////////


template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_view<Concrete_dim, Concrete_elem>& concrete_view) {
	static_assert(Opaque_dim <= Concrete_dim, "opaque dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Opaque_dim;
	
	using opaque_view_type = ndarray_view_opaque<Opaque_dim, ! std::is_const<Concrete_elem>::value>;
	using frame_ptr = typename opaque_view_type::frame_ptr;
	
	if(vw.is_null()) return opaque_view_type::null();
	
	Expects(vw.has_default_strides(Concrete_dim - frame_dim), "must have default strides within frame");
			
	const ndarray_format& array_format = tail_format<frame_dim>(vw);
	ndarray_opaque_frame_format opaque_frame_format(array_format);

	auto new_start = reinterpret_cast<frame_ptr>(vw.start());
	auto new_shape = head<Opaque_dim>(vw.shape());
	auto new_strides = head<Opaque_dim>(vw.strides());
	
	return opaque_view_type(new_start, new_shape, new_strides, opaque_frame_format);
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_view_opaque<Opaque_dim, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
) {
	if(vw.is_null()) return ndarray_view<Concrete_dim, Concrete_elem>::null();

	static_assert(std::is_const<Concrete_elem>::value || Opaque_mutable,
		"cannot cast const ndarray_view_opaque to mutable concrete ndarray_view");
	
	static_assert(Opaque_dim <= Concrete_dim,
		"opaque dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Opaque_dim;

	Assert(opaque_view.format().is_single_part(),
		"opaque view must be single-part for casting into concrete view");
	
	const ndarray_format& array_format = vw.format().array_format();

	Assert(array_format.elem_size() == sizeof(Elem),
		"opaque frame format has incorrect element type");
	Assert(is_multiple_of(array_format.elem_alignment(), alignof(Elem)),
		"opaque frame format has incorrect element type");
	Assert(frame_shape.product() * array_format.elem_stride() == array_format.frame_size(),
		"concrete frame shape must match opaque frame size and stride");
	
	ndptrdiff<frame_dim> concrete_frame_strides =
		ndarray_view<frame_dim, Concrete_elem>::default_strides(frame_shape, array_format.elem_padding());
	
	auto new_start = reinterpret_cast<Concrete_elem*>(vw.start());
	auto new_shape = ndcoord_cat(vw.shape(), frame_shape);
	auto new_strides = ndcoord_cat(vw.strides(), concrete_frame_strides);
		
	return ndarray_view<Concrete_dim, Concrete_elem>(new_start, new_shape, new_strides);
}


}
