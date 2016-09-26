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

#include "../nd/ndarray_view_cast.h"
#include "../flow/node_graph.h"
#include <algorithm>

namespace mf { namespace flow {


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem>
node_graph& filter_edge<Dim, Output_elem, Casted_elem, Input_elem>::this_node_graph() {
	Expects(node_input_ != nullptr);
	return node_input_->this_node().graph();
}

template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem>
void filter_edge<Dim, Output_elem, Casted_elem, Input_elem>::set_node_input(node_input& in) {
	Expects(node_input_ == nullptr);
	node_input_ = &in;
	if(is_connected_()) this->install_(this_node_graph());
}


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem>
void filter_edge<Dim, Output_elem, Casted_elem, Input_elem>::set_node_output(node_output& out, std::ptrdiff_t channel_index) {
	Expects(node_output_ == nullptr);
	node_output_ = &out;
	node_output_channel_index_ = channel_index;
	if(is_connected_()) this->install_(this_node_graph());
}


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem>
auto filter_edge<Dim, Output_elem, Casted_elem, Input_elem>::output_view_to_casted_view_
(const timed_frame_array_view& opaque_output_view) const -> casted_full_view_type {
	if(opaque_output_view.is_null()) return casted_full_view_type::null();

	auto concrete_output_view = from_opaque<Dim + 1, Output_elem>(
		extract_part(opaque_output_view, node_output_channel_index()),
		output_frame_shape()
	);
	return ndarray_view_cast<casted_full_view_type>(concrete_output_view);
}


///////////////


template<std::size_t Dim, typename Output_elem, typename Input_elem>
auto filter_direct_edge<Dim, Output_elem, Input_elem>::
cast_connected_node_output_view(const timed_frame_array_view& opaque_output_view) const -> input_full_view_type {
	// `generic_output_view` is reading from `node_output output_.this_node()`
	// its format is <Dim, Output_elem>; casting to <Dim, Input_elem>
	return base::output_view_to_casted_view_(opaque_output_view);
}


template<std::size_t Dim, typename Output_elem, typename Input_elem>
void filter_direct_edge<Dim, Output_elem, Input_elem>::install_(node_graph&, node_input& in, node_output& out) {
	in.connect(out);
}



///////////////


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
void filter_converting_edge<Dim, Output_elem, Casted_elem, Input_elem, Convert_function>::install_
(node_graph& gr, node_input& in, node_output& out) {
	sync_node& convert_node = gr.add_node<sync_node>();
	convert_node_.set_name("convert");
	convert_node_.set_handler(*this);
	auto& convert_node_input = convert_node_.add_input();
	auto& convert_node_output = convert_node_.output();
	convert_node_output_channel_ = &convert_node_->add_output_channel();

	convert_node_input.set_name("in");
	convert_node_output.set_name("out");
	convert_node_output_channel_->set_name("out");

	convert_node_input.connect(out);
	in.connect(convert_node_output);
}


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
void filter_converting_edge<Dim, Output_elem, Casted_elem, Input_elem, Convert_function>::
handler_setup(processing_node& convert_node) {
	std::size_t elem_count = base::output_frame_shape().product();
	ndarray_format frame_format = make_ndarray_format<Input_elem>(elem_count);
	
	convert_node.output_channel_at(0).define_frame_format(frame_format);
}


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
void filter_converting_edge<Dim, Output_elem, Casted_elem, Input_elem, Convert_function>::
handler_pre_process(processing_node& convert_node, processing_node_job& job) { }


template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
void filter_converting_edge<Dim, Output_elem, Casted_elem, Input_elem, Convert_function>::
handler_process(processing_node& convert_node, processing_node_job& job) {
	auto in = job.input_view(0)[0];
	const auto& out = job.output_view();
	
	auto concrete_in = from_opaque<Dim, Casted_elem>(
		extract_part(in, base::node_output_channel_index()),
		base::input_frame_shape()
	);
	auto concrete_out = from_opaque<Dim, Input_elem>(out, base::input_frame_shape());
	
	std::transform(concrete_in.begin(), concrete_in.end(), concrete_out.begin(), convert_function_);
}

template<std::size_t Dim, typename Output_elem, typename Casted_elem, typename Input_elem, typename Convert_function>
auto filter_converting_edge<Dim, Output_elem, Casted_elem, Input_elem, Convert_function>::
cast_connected_node_output_view(const timed_frame_array_view& generic_converted_view) const -> input_full_view_type {
	// `generic_output_view` is read from `convert_node_`.
	// its format is already <Dim, Input_elem>
	return from_opaque<Dim + 1, Input_elem>(generic_converted_view, base::input_frame_shape());
}


}}
