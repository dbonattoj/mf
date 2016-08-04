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


template<std::size_t Dim>
ndarray_view_generic<Dim>::ndarray_view_generic
(const frame_format& frm, byte* start, const generic_shape_type& shape, const generic_strides_type& strides) :
	base(start, ndcoord_cat(shape, frm.frame_size()), ndcoord_cat(strides, 1)),
	format_(frm) { }


template<std::size_t Dim>
ndarray_view_generic<Dim> ndarray_view_generic<Dim>::array_at(std::ptrdiff_t array_index) const {
	const frame_array_format& array_format = format().array_at(array_index);
	auto new_start = base::start() + array_format.offset();
	return ndarray_view_generic<Dim>(array_format, new_start, generic_shape(), generic_strides());
}


template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_view_generic<Generic_dim> to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw) {
	static_assert(Generic_dim <= Concrete_dim, "generic dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Generic_dim;
	
	if(vw.is_null()) return ndarray_view_generic<Generic_dim>::null();
	
	Expects(vw.has_default_strides(Concrete_dim - frame_dim), "must have default strides within frame");
			
	auto frm = tail_format<frame_dim>(vw);

	auto new_start = reinterpret_cast<byte*>(vw.start());
	auto new_shape = head<Generic_dim>(vw.shape());
	auto new_strides = head<Generic_dim>(vw.strides());
	
	return ndarray_view_generic<Generic_dim>(frm, new_start, new_shape, new_strides);
}


template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_view_generic<Generic_dim>& gen_vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape,
	std::ptrdiff_t array_index
) {
	static_assert(Generic_dim <= Concrete_dim, "generic dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Generic_dim;

	if(gen_vw.is_null()) return ndarray_view<Concrete_dim, Concrete_elem>::null();
	
	auto frm = gen_vw.format().array_at(array_index);

	MF_DEBUG_EXPR(frame_shape, frm.elem_stride(), frm.frame_size());


	Assert(frm.frame_size() == gen_vw.shape().back());
	Assert(frame_shape.product() * frm.elem_stride() == frm.frame_size());
	
	ndptrdiff<frame_dim> concrete_frame_strides =
		ndarray_view<frame_dim, Concrete_elem>::default_strides(frame_shape, frm.elem_padding());
	
	auto new_start = reinterpret_cast<Concrete_elem*>(gen_vw.start() + frm.offset());
	auto new_shape = ndcoord_cat(gen_vw.generic_shape(), frame_shape);
	auto new_strides = ndcoord_cat(gen_vw.generic_strides(), concrete_frame_strides);
		
	return ndarray_view<Concrete_dim, Concrete_elem>(new_start, new_shape, new_strides);
}

}
