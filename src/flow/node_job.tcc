/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

namespace mf { namespace flow {

template<typename Output>
decltype(auto) node_job::out(Output& port) {
	constexpr std::size_t dimension = Output::dimension;
	using elem_type = typename Output::elem_type;
	
	using view_type = ndarray_view<dimension, elem_type>;
	
	output_view_handle_type* ptr = outputs_map_.at(port.index());
	MF_ASSERT(ptr != nullptr);
	
	return from_generic<dimension, elem_type>(
		ptr->second,
		port.frame_shape()
	);
}


template<typename Input>
decltype(auto) node_job::in_full(Input& port) {
	constexpr std::size_t dimension = Input::dimension;
	using elem_type = typename Input::elem_type;
	
	using view_type = ndarray_timed_view<dimension + 1, elem_type>;
	
	input_view_handle_type* ptr = inputs_map_.at(port.index());
	if(! ptr) return view_type::null();
	
	return from_generic<dimension + 1, elem_type>(
		ptr->second,
		port.frame_shape()
	);
}


template<typename Input>
decltype(auto) node_job::in(Input& port) {
	constexpr std::size_t dimension = Input::dimension;
	using elem_type = typename Input::elem_type;

	using full_view_type = ndarray_timed_view<dimension + 1, elem_type>;
	using view_type = ndarray_view<dimension, elem_type>;

	auto full_view = in_full(port);
	if(full_view) return full_view.at_time(time_);
	return view_type::null();
}


template<typename Param>
decltype(auto) node_job::param(const Param& param) {
	return param.get(time_);
}

	
}}
