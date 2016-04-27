namespace mf {

template<typename Allocator>
ndarray_generic<Allocator>::ndarray_generic
(const frame_properties& frame_prop, std::size_t frame_count, std::size_t padding, const Allocator& allocator) :
base(
	make_ndsize(frame_count, frame_prop.length),
	padding,
	frame_prop.alignment,
	allocator
) {
	if(frame_prop.length % frame_prop.alignment != 0)
		throw std::invalid_argument("frame length must be multiple of alignment");
	if(padding % frame_prop.alignment != 0)
		throw std::invalid_argument("frame padding must be multiple of alignment");
}

}
