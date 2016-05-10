namespace mf {


template<typename Allocator>
ndarray_generic<Allocator>::ndarray_generic(
	const ndarray_generic_properties& prop,
	std::size_t padding,
	const Allocator& allocator
) :
base(
	make_ndsize(prop.array_length(), prop.frame_length()),
	padding,
	prop.format().alignment(),
	allocator
),
format_(prop.format()) { }


}
