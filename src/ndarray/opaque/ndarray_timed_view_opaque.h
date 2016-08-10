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

#ifndef MF_NDARRAY_TIMED_VIEW_OPAQUE_H_
#define MF_NDARRAY_TIMED_VIEW_OPAQUE_H_

#include "ndarray_view_opaque.h"
#include "../detail/ndarray_timed_view_derived.h"

namespace mf {


/// \ref ndarray_view_opaque with absolute time indices associated to first (opaque) dimension.
template<std::size_t Dim, bool Mutable = true>
using ndarray_timed_view_opaque = detail::ndarray_timed_view_derived<ndarray_view_opaque<Dim, Mutable>>;



template<std::size_t Dim, bool Mutable>
ndarray_timed_view_opaque<Dim, Mutable> extract_part
(const ndarray_timed_view_opaque<Dim, Mutable>& vw, std::ptrdiff_t part_index) {
	auto non_timed_vw = extract_part(vw.non_timed(), part_index);
	return ndarray_timed_view_opaque<Dim, Mutable>(non_timed_vw, vw.start_time());
}



template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_timed_view<Concrete_dim, Concrete_elem>& concrete_view) {
	auto non_timed_opaque = to_opaque(concrete_view.non_timed());
	return ndarray_timed_view<Concrete_dim, Concrete_elem>(non_timed_opaque, concrete_view.start_time());
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_timed_view_opaque<Opaque_dim, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
) {
	auto non_timed_concrete = from_opaque<Concrete_dim, Concrete_elem>(opaque_view.non_timed());
	return ndarray_timed_view_opaque<Opaque_dim, Opaque_mutable>(non_timed_concrete, opaque_view.start_time());
}


}

#endif
