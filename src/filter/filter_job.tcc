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

template<std::size_t Dim, typename Elem>
ndarray_timed_view<Dim + 1, Elem> filter_job::in_full(filter_input<Dim, Elem>& pt) {
	std::ptrdiff_t index = pt.index();
	if(! node_job_.has_input_view(index))
		return ndarray_timed_view<Dim + 1, Elem>();
			
	timed_frame_array_view gen_vw = node_job_.input_view(index);
	return from_opaque<Dim + 1, Elem>(
		gen_vw,
		pt.frame_shape()
	);
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> filter_job::in(filter_input<Dim, Elem>& pt) {
	return in(pt, time());
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> filter_job::in(filter_input<Dim, Elem>& pt, time_unit t) {
	auto full_vw = in_full(pt);
	if(full_vw) return full_vw.at_time(t);
	return decltype(full_vw[0])();
}


template<std::size_t Dim, typename Elem>
ndarray_view<Dim, Elem> filter_job::out(filter_output<Dim, Elem>& pt) {
	std::ptrdiff_t index = pt.index();
	
	auto gen_vw = extract_part(node_job_.output_view(), index);
	return from_opaque<Dim, Elem>(
		gen_vw,
		pt.frame_shape()
	);
}


template<typename Value>
Value filter_job::in(const filter_extern_parameter<Value>& extern_param) {
	return in(extern_param, time());
}


template<typename Value>
Value filter_job::in(const filter_extern_parameter<Value>& extern_param, time_unit t) {
	if(extern_param.linked_parameter().is_deterministic()) {
		return extern_param.linked_parameter().deterministic_value(t);
	} else {
		// TODO handle not availble
		const node_parameter_value& value = node_job_.input_parameter(extern_param.linked_parameter().id(), t);
		return value.get<Value>();
	}
}


// TODO allow returning deterministic by reference

template<typename Value>
Value filter_job::param(const filter_parameter<Value>& param) {
	// read parameter of this node
	if(param.is_deterministic()) {
		return param.deterministic_value(time());
	} else {
		Assert(node_job_.has_parameter(param.id()));
		const node_parameter_value& value = node_job_.parameter(param.id());
		return value.get<Value>();
	}
}


template<typename Value>
void filter_job::set_param(const filter_parameter<Value>& param, const Value& new_value) {
	Assert(param.is_dynamic());
	node_parameter_value& val = node_job_.parameter(param.id());
	val.get<Value>() = new_value;
}


template<typename Value>
void filter_job::send_param(const filter_extern_parameter<Value>& extern_param, const Value& new_value) {
	node_parameter_id id = extern_param.linked_parameter().id();
	node_parameter_value new_val(new_value);
	node_job_.send_parameter(id, new_val);
}


}}

