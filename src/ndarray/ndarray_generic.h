#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "../common.h"
#include "ndarray.h"
#include "ndarray_view.h"
#include "ndarray_timed_view.h"
#include "../elem.h"
#include "../utility/memory.h"
#include <cstdint>

namespace mf {

using ndarray_view_generic = ndarray_view<2, byte>;
using ndarray_frame_view_generic = ndarray_view<1, byte>;

using ndarray_timed_view_generic = ndarray_timed_view<2, byte>;

template<std::size_t Dim, typename Elem>
ndarray_frame_view_generic to_generic_frame(const ndarray_view<Dim, Elem>& view) {
	std::size_t length = view.shape().product() * view.strides().back();
	byte* data = reinterpret_cast<byte*>(view.start());
	return ndarray_frame_view_generic(data, make_ndsize(length));
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> from_generic_frame(const ndarray_frame_view_generic& view, const ndsize<Dim>& shape) {
	if(shape.product() * sizeof(Elem) != view.size())
		throw std::invalid_argument("shape does not match size of generic frame view");
	Elem* ptr = reinterpret_cast<Elem*>(view.start());
	return ndarray_view<Dim, Elem>(ptr, shape);
}


template<std::size_t Dim, typename Elem>
ndarray_view_generic to_generic(const ndarray_view<Dim, Elem>& view) {
	std::size_t frame_length = view.shape().tail().product() * view.strides().back();
	std::size_t frame_count = view.shape().front();
	std::ptrdiff_t frame_stride = view.strides().front();
	byte* data = reinterpret_cast<byte*>(view.start());
	return ndarray_view_generic(data, make_ndsize(frame_count, frame_length), make_ndptrdiff(frame_stride, 1));
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> from_generic(const ndarray_view_generic& view, const ndsize<Dim - 1>& frame_shape) {
	if(frame_shape.product() * sizeof(Elem) != view.shape().tail().product())
		throw std::invalid_argument("shape does not match size of generic view");
	std::size_t frame_count = view.shape().front();
	std::ptrdiff_t frame_stride = view.strides().front();
	Elem* ptr = reinterpret_cast<Elem*>(view.start());
	return ndarray_view<Dim, Elem>(
		ptr,
		ndcoord_cat(make_ndsize(frame_count), frame_shape),
		ndcoord_cat(make_ndsize(frame_stride), ndarray_view<Dim - 1, Elem>::default_strides())
	);
}


struct frame_properties {
	std::size_t length = 0;
	std::size_t alignment = 1;
};


template<typename Allocator = raw_allocator>
class ndarray_generic : public ndarray<2, byte, Allocator> {
	using base = ndarray<2, byte, Allocator>;
	
public:
	ndarray_generic(const frame_properties&, std::size_t n, std::size_t padding = 0, const Allocator& = Allocator());
	explicit ndarray_generic(const ndarray_generic&) = default;
};


}

#include "ndarray_generic.tcc"

#endif
