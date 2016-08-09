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

#ifndef MF_NDARRAY_TIMED_VIEW_GENERIC_H_
#define MF_NDARRAY_TIMED_VIEW_GENERIC_H_

#include "ndarray_view_generic.h"
#include "../detail/ndarray_timed_view_derived.h"

namespace mf {


/// \ref ndarray_view_generic with absolute time indices associated to first (generic) dimension.
template<std::size_t Dim, bool Mutable = true>
using ndarray_timed_view_generic = detail::ndarray_timed_view_derived<ndarray_view_generic<Dim, Mutable>>;



template<std::size_t Dim, bool Mutable>
ndarray_timed_view_generic<Dim, Mutable> extract_array
(const ndarray_timed_view_generic<Dim, Mutable>& vw, std::ptrdiff_t array_index) {
	auto non_timed_vw = extract_array(vw.non_timed(), array_index);
	return ndarray_timed_view_generic<Dim, Mutable>(non_timed_vw, vw.start_time());
}




template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_timed_view_generic<Generic_dim> to_generic(const ndarray_timed_view<Concrete_dim, Concrete_elem>& vw) {
	auto gen_vw = to_generic<Generic_dim>(vw.non_timed());
	return ndarray_timed_view_generic<Generic_dim>(gen_vw, vw.start_time());
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_timed_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_timed_view_generic<Generic_dim>& gen_vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape,
	std::ptrdiff_t array_index = 0
) {
	auto vw = from_generic<Concrete_dim, Concrete_elem, Generic_dim>(gen_vw.non_timed(), frame_shape, array_index);
	return ndarray_timed_view<Concrete_dim, Concrete_elem>(vw, gen_vw.start_time());
}


}

#endif
