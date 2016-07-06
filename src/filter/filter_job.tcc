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
