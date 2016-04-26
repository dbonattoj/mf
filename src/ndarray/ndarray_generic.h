#ifndef MF_NDARRAY_GENERIC_H_
#define MF_NDARRAY_GENERIC_H_

#include "../common.h"
#include "ndarray_view.h"
#include "ndarray_timed_view.h"
#include "elem.h"
#include <cstdint>

namespace mf {

using ndarray_generic_view = ndarray_view<2, byte>;
using ndarray_generic_frame_view = ndarray_view<1, byte>;

using ndarray_generic_timed_view = ndarray_timed_view<2, byte>;

template<std::size_t Dim, typename Elem>
ndarray_generic_frame_view to_generic_frame(const ndarray_view<Dim, Elem>& view) {
	std::size_t length = view.shape().product() * view.strides().back();
	byte* data = reinterpret_cast<byte*>(view.start());
	return ndarray_generic_view(data, make_ndsize(length));
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> from_generic_frame(const ndarray_frame_view& view, const ndsize<Dim>& shape) {
	if(shape.product() * sizeof(Elem) != view.size())
		throw std::invalid_argument("shape does not match size of generic frame view");
	Elem* ptr = reinterpret_cast<Elem*>(view.start());
	return ndarray_view<Dim, Elem>(ptr, shape);
}


template<std::size_t Dim, typename Elem>
ndarray_array_view to_generic(const ndarray_view<Dim, Elem>& view) {
	std::size_t frame_length = view.shape().tail().product() * view.strides().back();
	std::size_t frame_count = view.shape().front();
	std::ptrdiff_t frame_stride = view.strides().front();
	byte* data = reinterpret_cast<byte*>(view.start());
	return ndarray_generic_view(data, make_ndsize(frame_count, frame_length), make_ndptrdiff(frame_stride, 1));
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> from_generic(const ndarray_generic_view& view, const ndsize<Dim - 1>& frame_shape) {
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


template<typename Allocator = raw_allocator>
class ndarray_generic_array : ndarray<2, byte> {
public:
	struct frame_properties {
		std::size_t count = 0;
		std::size_t length = 0;
		std::size_t padding = 0;
		std::size_t alignment = 1;
	};

	explicit ndarray_generic_array(const frame_properties&, const Allocator& = Allocator());
	explicit ndarray(const ndarray_generic_array&) = default;
};


}

#include "ndarray_generic.tcc"

#endif
