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

#include "../../debug.h"

namespace mf {

template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_view_generic<Generic_dim> to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw) {
	static_assert(Generic_dim <= Concrete_dim, "generic dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Generic_dim;
	
	if(vw.is_null()) return ndarray_view_generic<Generic_dim>::null();
	
	Expects(vw.has_default_strides(frame_dim), "must have default strides within frame");
			
	std::size_t frame_size = vw.shape().template tail<frame_dim>().product() * sizeof(Concrete_elem);
	auto frm = format(vw);
		
	auto generic_start = reinterpret_cast<byte*>(vw.start());
	auto generic_shape = ndcoord_cat(vw.shape().template head<Generic_dim>(), frame_size);
	auto generic_strides = ndcoord_cat(vw.strides().template head<Generic_dim>(), 1);
	
	return ndarray_view_generic<Generic_dim>(generic_start, frm, generic_shape, generic_strides);
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
	Assert(frm.frame_size() == gen_vw.shape().back());
	
	ndptrdiff<frame_dim> concrete_frame_strides =
		ndarray_view<frame_dim, Concrete_elem>::default_strides(frame_shape, frm.padding());
	
	auto concrete_start = reinterpret_cast<Concrete_elem*>(vw.start() + frm.offset());
	auto concrete_shape = ndcoord_cat(vw.shape().template head<Generic_dim>(), frame_shape);
	auto concrete_strides = ndcoord_cat(vw.strides().template head<Generic_dim>(), concrete_frame_strides);
		
	return ndarray_view<Concrete_dim, Concrete_elem>(concrete_start, concrete_shape, concrete_strides);
}

}
