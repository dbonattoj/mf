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
	return from_opaque<dimension + 1, elem_type>(
		gen_vw,
		pt.frame_shape()
	);
}


template<typename Input> decltype(auto) filter_job::in(Input& pt) {
	return in(node_job_.time());
}


template<typename Input> decltype(auto) filter_job::in(Input& pt, time_unit t) {
	auto full_vw = in_full(pt);
	if(full_vw) return full_vw.at_time(t);
	return full_vw.null();
}


template<typename Output> decltype(auto) filter_job::out(Output& pt) {
	constexpr std::size_t dimension = Output::dimension;
	using elem_type = typename Output::elem_type;
	
	std::ptrdiff_t index = pt.index();
	
	auto gen_vw = extract_part(node_job_.output_view(), index);
	return from_opaque<dimension, elem_type>(
		gen_vw,
		pt.frame_shape()
	);
}

// TODO allow returning deterministic by reference

template<typename Value>
Value filter_job::param(const filter_parameter<Value>& param) {
	// read parameter of this node
	if(param.is_deterministic()) {
		return param.deterministic_value(time());
	} else {
		Assert(node_job_.has_parameter(id()));
		const node_parameters_value& value = node_job_.parameter(id());
		return value.get<Value>();
	}
}


template<typename Value>
Value filter_job::param(const filter_extern_parameter<Value>& extern_param) {
	return param(extern_param, time());
}


template<typename Value>
Value filter_job::param(const filter_extern_parameter<Value>& extern_param, time_unit t) {
	if(extern_param.linked_parameter().is_deterministic()) {
		return extern_param.linked_parameter().deterministic_value(t);
	} else {
		
	}
}


template<typename Value>
void filter_job::update_param(filter_extern_parameter<Value>& extern_param, const Value& new_value) {
	
}



template<typename Param> decltype(auto) filter_job::param(Param& param) {
	return param.get(time());
}


}}

