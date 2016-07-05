namespace mf { namespace flow {

template<typename Input> decltype(auto) filter_job::in_full(Input& pt) {
	constexpr std::size_t dimension = Input::dimension;
	using elem_type = typename Input::elem_type;
			
	std::ptrdiff_t index = pt.index();
	if(! node_job_.has_input_view(index))
		return ndarray_timed_view<dimension + 1, elem_type>();
			
	timed_frame_array_view gen_vw = node_job_.input_view(index);
	return from_generic<dimension + 1, elem_type>(
		gen_vw,
		pt.frame_shape()
	);
}


template<typename Input> decltype(auto) filter_job::in(Input& pt) {
	auto full_vw = in_full(pt);
	if(full_vw) return full_vw.at_time(node_job_.time());
	return decltype(full_vw[0])();
}


template<typename Output> decltype(auto) filter_job::out(Output& pt) {
	constexpr std::size_t dimension = Output::dimension;
	using elem_type = typename Output::elem_type;
	
	frame_view gen_vw = node_job_.output_view();
	return from_generic<dimension, elem_type>(
		gen_vw,
		pt.frame_shape()
	);
}


}}
