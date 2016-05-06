namespace mf { namespace flow {

template<typename Output>
auto node_job::out(Output& port) {
	constexpr std::size_t dimension = Output::dimension;
	using elem_type = typename Output::elem_type;
	
	using view_type = ndarray_view<dimension, elem_type>;
	
	output_view_handle_type* ptr = outputs_map_.at(port.index());
	MF_ASSERT(ptr != nullptr);
	
	return from_generic_frame<dimension, elem_type>(
		ptr->second,
		port.frame_shape()
	);
}


template<typename Input>
auto node_job::in_full(Input& port) {
	constexpr std::size_t dimension = Input::dimension;
	using elem_type = typename Input::elem_type;
	
	using view_type = ndarray_timed_view<dimension + 1, elem_type>;
	
	input_view_handle_type* ptr = inputs_map_.at(port.index());
	if(! ptr) return view_type::null();
	
	return from_generic_timed<dimension, elem_type>(
		ptr->second,
		port.frame_shape()
	);
}


template<typename Input>
auto node_job::in(Input& port) {
	constexpr std::size_t dimension = Input::dimension;
	using elem_type = typename Input::elem_type;

	using full_view_type = ndarray_timed_view<dimension + 1, elem_type>;
	using view_type = ndarray_view<dimension, elem_type>;

	auto full_view = in_full(port);
	if(full_view) return full_view.at_time(time_);
	return view_type::null();
}


template<typename Param>
auto node_job::param(const Param& param) {
	return param.get(time__);
}

	
}}
