#ifndef MF_NDARRAY_VIEW_CAST_H_
#define MF_NDARRAY_VIEW_CAST_H_

#include "ndarray_view.h"
#include "../common.h"
#include "../elem.h"
#include "../elem_tuple.h"
#include "../utility/misc.h"

namespace mf {
	
namespace detail {
	template<typename Output_view, typename Input_view>
	struct ndarray_view_caster;
	
	// single element from tuple
	template<typename Output_elem, std::size_t Dim, typename... Input_elems>
	struct ndarray_view_caster<
		ndarray_view<Dim, Output_elem>,
		ndarray_view<Dim, elem_tuple<Input_elems...>>
	>{
		using input_tuple_type = elem_tuple<Input_elems...>;
		
		using output_view_type = ndarray_view<Dim, Output_elem>;
		using input_view_type = ndarray_view<Dim, input_tuple_type>;
						
		output_view_type operator()(const input_view_type& arr) const {
			constexpr std::ptrdiff_t index = elem_tuple_index<Output_elem, input_tuple_type>;
			constexpr std::ptrdiff_t offset = elem_tuple_offset<index, input_tuple_type>;

			auto* start = reinterpret_cast<Output_elem*>(
				advance_raw_ptr(arr.start(), offset)
			);
			return output_view_type(
				start,
				arr.shape(),
				arr.strides()
			);
		}
	};
	
	// scalars from elem
	template<std::size_t Dim, typename Input_elem>
	struct ndarray_view_caster<
		ndarray_view<Dim + 1, typename elem_traits<Input_elem>::scalar_type>,
		ndarray_view<Dim, Input_elem>
	>{
		using elem_traits_type = elem_traits<Input_elem>;
		using elem_scalar_type = typename elem_traits_type::scalar_type;
		
		using output_view_type = ndarray_view<Dim + 1, elem_scalar_type>;
		using input_view_type = ndarray_view<Dim, Input_elem>;
		
		output_view_type operator()(const input_view_type& arr) const {
			auto* start = reinterpret_cast<elem_scalar_type*>(arr.start());
			return output_view_type(
				start,
				ndcoord_cat(arr.shape(), make_ndsize(elem_traits_type::components)),
				ndcoord_cat(arr.strides(), make_ndptrdiff(elem_traits_type::stride))
			);
		}
	};

}


template<typename Output_view, typename Input_view>
Output_view ndarray_view_cast(const Input_view& view) {
	detail::ndarray_view_caster<Output_view, Input_view> caster;
	return caster(view);
}


template<typename Output_view, typename Input_view>
Output_view ndarray_view_reinterpret_cast(const Input_view& in_view) {
	using in_elem_type = typename Input_view::value_type;
	using out_elem_type = typename Output_view::value_type;
	static_assert(Output_view::dimension == Input_view::dimension, "output and input view must have same dimension");
	std::ptrdiff_t in_stride = in_view.strides().back();
	if(in_stride < sizeof(out_elem_type))
		throw std::invalid_argument("output ndarray_view elem type is too large");
	if(in_stride % alignof(out_elem_type) != 0)
		throw std::invalid_argument("output ndarray_view elem type has incompatible alignment");
	
	auto new_start = reinterpret_cast<out_elem_type*>(in_view.start());
	return Output_view(new_start, in_view.shape(), in_view.strides());
}



}

#endif

