namespace mf {

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
ndarray_view_generic to_generic(const ndarray_view<Dim + 1, Elem>& view) {
	std::size_t frame_length = view.shape().tail().product() * view.strides().back();
	std::size_t frame_count = view.shape().front();
	std::ptrdiff_t frame_stride = view.strides().front();
	byte* data = reinterpret_cast<byte*>(view.start());
	return ndarray_view_generic(data, make_ndsize(frame_count, frame_length), make_ndptrdiff(frame_stride, 1));
}


template<std::size_t Dim, typename Elem>
ndarray_timed_view_generic to_generic_timed(const ndarray_timed_view<Dim + 1, Elem>& view) {
	return ndarray_timed_view_generic(to_generic(view), view.start_time());
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim + 1, Elem> from_generic(const ndarray_view_generic& view, const ndsize<Dim>& frame_shape) {
	if(view.shape().front() != 0 && (frame_shape.product() * sizeof(Elem) != view.shape().tail().product()))
		throw std::invalid_argument("frame shape does not match size of generic view");
	std::size_t frame_count = view.shape().front();
	std::ptrdiff_t frame_stride = view.strides().front();
	Elem* ptr = reinterpret_cast<Elem*>(view.start());
	return ndarray_view<Dim + 1, Elem>(
		ptr,
		ndcoord_cat(make_ndsize(frame_count), frame_shape),
		ndcoord_cat(make_ndptrdiff(frame_stride), ndarray_view<Dim, Elem>::default_strides(frame_shape))
	);
}


template<std::size_t Dim, typename Elem>
ndarray_timed_view<Dim + 1, Elem> from_generic_timed(const ndarray_timed_view_generic& view, const ndsize<Dim>& frame_shape) {
	return ndarray_timed_view<Dim + 1, Elem>(from_generic<Dim, Elem>(view, frame_shape), view.start_time());
}


template<typename Allocator>
ndarray_generic<Allocator>::ndarray_generic
(const frame_array_properties& frame_properties, std::size_t frame_padding, const Allocator& allocator) :
base(
	make_ndsize(frame_properties.array_length, frame_properties.frame_size()),
	frame_padding,
	frame_properties.alignment(),
	allocator
) {
	MF_EXPECTS(padding % frame_properties.alignment() == 0);
}


}