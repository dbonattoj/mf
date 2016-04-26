namespace mf {

template<typename Allocator>
ndarray_generic_array::ndarray_generic_array(const frame_properties& frame_prop, const Allocator& allocator) :
ndarray<2, byte>(
	make_ndsize(frame_prop.count, frame_prop.length),
	frame_prop.padding,
	frame_prop.alignment,
	allocator
) {
	if(frame_prop.length % frame_prop.alignment != 0)
		throw std::invalid_argument("frame length must be multiple of alignment");
	if(frame_prop.padding % frame_prop.alignment != 0)
		throw std::invalid_argument("frame padding must be multiple of alignment");
}

}
