#ifndef MF_NDARRAY_VIEW_OPAQUE_CAST_H_
#define MF_NDARRAY_VIEW_OPAQUE_CAST_H_

#include "opaque_format_array.h"
#include "opaque_format_multi_array.h"
#include "ndarray_view_opaque.h"
#include "ndarray_timed_view_opaque.h"
#include "ndarray_opaque.h"

namespace mf {

/// Cast from \ref ndarray_view to opaque \ref ndarray_view_opaque with given dimension.
/** Opaque view has \ref opaque_format_array format. */
template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_view<Concrete_dim, Concrete_elem>& concrete_view);



/// Cast from \ref ndarray_view_opaque to concrete \ref ndarray_view with given dimension, frame shape and element type.
/** Opaque view has \ref opaque_format_array format. */
template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_view_opaque<Opaque_dim, opaque_format_array, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
);


/// Cast from \ref ndarray_view_opaque with multi-array format to single-array view to one part.
template<std::size_t Dim, bool Mutable>
auto extract_part(const ndarray_view_opaque<Dim, opaque_format_multi_array, Mutable>&, std::ptrdiff_t part_index);



template<std::size_t Dim, bool Mutable>
auto extract_part(const ndarray_timed_view_opaque<Dim, opaque_format_multi_array, Mutable>& vw, std::ptrdiff_t part_index) {
	auto non_timed_vw = extract_part(vw.non_timed(), part_index);
	return ndarray_timed_view_opaque<Dim, opaque_format_array, Mutable>(non_timed_vw, vw.start_time());
}



template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_timed_view<Concrete_dim, Concrete_elem>& concrete_view) {
	auto non_timed_opaque = to_opaque(concrete_view.non_timed());
	constexpr bool opaque_mutable = ! std::is_const<Concrete_elem>::value;
	using opaque_view_type = ndarray_timed_view_opaque<Opaque_dim, opaque_format_array, opaque_mutable>;
	return opaque_view_type(non_timed_opaque, concrete_view.start_time());
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_timed_view_opaque<Opaque_dim, opaque_format_array, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
) {
	auto non_timed_concrete = from_opaque<Concrete_dim, Concrete_elem>(opaque_view.non_timed(), frame_shape);
	return ndarray_timed_view<Concrete_dim, Concrete_elem>(non_timed_concrete, opaque_view.start_time());
}


template<std::size_t Dim>
auto extract_part(ndarray_opaque<Dim, opaque_format_multi_array>& arr, std::ptrdiff_t part_index) {
	return extract_part(arr.view(), part_index);
}


template<std::size_t Dim>
auto extract_part(const ndarray_opaque<Dim, opaque_format_multi_array>& arr, std::ptrdiff_t part_index) {
	return extract_part(arr.cview(), part_index);
}


}

#include "ndarray_view_opaque_cast.tcc"

#endif
