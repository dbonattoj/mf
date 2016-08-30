#include "ndarray_view_opaque.h"
#include "../ndarray_view.h"

namespace mf {

template<std::size_t Opaque_dim, std::size_t Concrete_dim, typename Concrete_elem>
auto to_opaque(const ndarray_view<Concrete_dim, Concrete_elem>& concrete_view) {
	static_assert(Opaque_dim <= Concrete_dim,
		"opaque dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Opaque_dim;
	
	using opaque_view_type = ndarray_view_opaque<Opaque_dim, ndarray_opaque_format_array, ! std::is_const<Concrete_elem>::value>;
	using frame_ptr = typename ndarray_opaque_format_array::frame_ptr;
	
	if(concrete_view.is_null()) return opaque_view_type::null();
	
	Assert(concrete_view.has_default_strides(Concrete_dim - frame_dim),
		"must have default strides within frame");
			
	const ndarray_format& array_format = tail_format<frame_dim>(concrete_view);
	ndarray_opaque_frame_format opaque_frame_format(array_format);

	auto new_start = reinterpret_cast<frame_ptr>(concrete_view.start());
	auto new_shape = head<Opaque_dim>(concrete_view.shape());
	auto new_strides = head<Opaque_dim>(concrete_view.strides());
	
	return opaque_view_type(new_start, new_shape, new_strides, opaque_frame_format);
}



template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Opaque_dim, bool Opaque_mutable>
auto from_opaque(
	const ndarray_view_opaque<Opaque_dim, ndarray_opaque_format_array, Opaque_mutable>& opaque_view,
	const ndsize<Concrete_dim - Opaque_dim>& frame_shape
) {
	if(opaque_view.is_null()) return ndarray_view<Concrete_dim, Concrete_elem>::null();

	static_assert(std::is_const<Concrete_elem>::value || Opaque_mutable,
		"cannot cast const ndarray_view_opaque to mutable concrete ndarray_view");
	
	static_assert(Opaque_dim <= Concrete_dim,
		"opaque dimension must be lower or equal to concrete dimension");
	constexpr std::size_t frame_dim = Concrete_dim - Opaque_dim;

	Assert(opaque_view.format().is_single_part(),
		"opaque view must be single-part for casting into concrete view");
	
	const ndarray_format& array_format = opaque_view.format().array_format();

	Assert(array_format.elem_size() == sizeof(Concrete_elem),
		"opaque frame format has incorrect element type");
	Assert(is_multiple_of(array_format.elem_alignment(), alignof(Concrete_elem)),
		"opaque frame format has incorrect element type");	
	Assert(frame_shape.product() * array_format.stride() == array_format.frame_size(),
		"concrete frame shape must match opaque frame size and stride");
	
	ndptrdiff<frame_dim> concrete_frame_strides =
		ndarray_view<frame_dim, Concrete_elem>::default_strides(frame_shape, array_format.elem_padding());
	
	auto new_start = reinterpret_cast<Concrete_elem*>(opaque_view.start());
	auto new_shape = ndcoord_cat(opaque_view.shape(), frame_shape);
	auto new_strides = ndcoord_cat(opaque_view.strides(), concrete_frame_strides);
		
	return ndarray_view<Concrete_dim, Concrete_elem>(new_start, new_shape, new_strides);
}

}
