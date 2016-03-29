#ifndef MF_NDARRAY_VIEW_CAST_H_
#define MF_NDARRAY_VIEW_CAST_H_

#include "ndarray_view.h"
#include "../elem.h"
#include "../utility/misc.h"

namespace mf {


template<std::size_t Output_view,, std::size_t Input_dim, typename Input_t>
Output_view ndarray_view_cast(const ndarray_view<Input_dim, Input_t>&) {
	static_assert(false, "unsupported ndarray_view_cast");
}


template<typename Output_elem, std::size_t Dim, typename Input_elems...>
auto ndarray_view_cast<
	ndarray_view<Dim, Output_elem>
>
(const ndarray_view<Dim, const elem_tuple<Input_elems...>>& arr) {
	using result_type = ndarray_view<Dim, Output_elem>;
	return result_type(
		advance_raw_ptr<Output_elem>(arr.start(), arr.offset<Output_elem>()),
		arr.shape(),
		arr.strides()
	);
}



template<std::size_t Dim, typename Input_elem>
auto ndarray_view_cast<
	ndarray_view<Dim + 1, typename elem_traits<Input_elem>::scalar>
>
(const ndarray_view<Dim, const Input_elem& arr) {
	using result_type = ndarray_view<Dim + 1, typename elem_traits<Input_elem>::scalar>;
	return result_type(
		advance_raw_ptr<scalar_type>(arr.start(), 0)
		ndcoord_cat(arr.shape(), make_ndsize(traits_type::dimension)),
		ndcoord_cat(arr.strides(), make_ndptrdiff(traits_type::stride))
	);
}





}

#endif

