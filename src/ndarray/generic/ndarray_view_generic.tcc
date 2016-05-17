#include "../../debug.h"

namespace mf {

template<std::size_t Generic_dim, std::size_t Concrete_dim, typename Concrete_elem>
ndarray_view_generic<Generic_dim> to_generic(const ndarray_view<Concrete_dim, Concrete_elem>& vw) {
	static_assert(Generic_dim <= Concrete_dim, "generic dimension must be lower of equal");
	constexpr std::size_t frame_dim = Concrete_dim - Generic_dim;
	
	if(vw.is_null()) return ndarray_view_generic<Generic_dim>::null(); // passthrough null view
	
	if(! vw.has_default_strides(frame_dim)) throw std::invalid_argument("must have default strides within frame");
		
	std::size_t frame_length = vw.shape().template tail<frame_dim>().product();
	frame_format format = frame_format::default_format<Concrete_elem>(vw.strides().back());
	
	auto generic_start = reinterpret_cast<byte*>(vw.start());
	auto generic_shape = ndcoord_cat(vw.shape().template head<Generic_dim>(), frame_length);
	auto generic_strides = ndcoord_cat(vw.strides().template head<Generic_dim>(), format.stride());
	
	return ndarray_view_generic<Generic_dim>(generic_start, format, generic_shape, generic_strides);
}


template<std::size_t Concrete_dim, typename Concrete_elem, std::size_t Generic_dim>
ndarray_view<Concrete_dim, Concrete_elem> from_generic(
	const ndarray_view_generic<Generic_dim>& vw,
	const ndsize<Concrete_dim - Generic_dim>& frame_shape)
{
	static_assert(Generic_dim <= Concrete_dim, "generic dimension must be lower of equal");
	constexpr std::size_t frame_dim = Concrete_dim - Generic_dim;

	if(vw.is_null()) return ndarray_view<Concrete_dim, Concrete_elem>::null(); // passthrough null view
	
	if(! vw.format().template is_type<Concrete_elem>()) throw std::invalid_argument("elem type mismatch");	
	if(frame_shape.product() != vw.shape().back()) throw std::invalid_argument("frame shape mismatch");
	
	ndptrdiff<frame_dim> concrete_frame_strides =
		ndarray_view<frame_dim, Concrete_elem>::default_strides(frame_shape, vw.format().padding());
	
	auto concrete_start = reinterpret_cast<Concrete_elem*>(vw.start());
	auto concrete_shape = ndcoord_cat(vw.shape().template head<Generic_dim>(), frame_shape);
	auto concrete_strides = ndcoord_cat(vw.strides().template head<Generic_dim>(), concrete_frame_strides);
		
	return ndarray_view<Concrete_dim, Concrete_elem>(concrete_start, concrete_shape, concrete_strides);
}

}
