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

#include "../flow/node.h"
#include "../flow/processing_node.h"
#include "../flow/multiplex_node.h"
#include "../flow/graph.h"

namespace mf { namespace flow {

template<std::size_t Output_dim, typename Output_elem>
filter_output<Output_dim, Output_elem>::filter_output(filter& filt) {
	filt.register_output(*this);
}


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::edge_has_connected(edge_base_type& edge) {
	edges_.push_back(&edge);
}


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::install(processing_node& nd) {
	Assert(edges_.size() == 1, "installing filter output with multiple edges requires multiplex node");
	node_output_channel_ = &nd.add_output_channel();
	edges_.front()->set_node_output(nd.output(), node_output_channel_->index());
}


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::install(processing_node& nd, multiplex_node& mpx_nd) {
	node_output_channel_ = &nd.add_output_channel();
	
	for(edge_base_type* edge : edges_) {
		std::ptrdiff_t channel_index = node_output_channel_->index();
		multiplex_node_output& mpx_out = mpx_nd.add_output(channel_index);
		edge->set_node_output(mpx_out, 0);
	}
}



template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::define_frame_shape(const frame_shape_type& shp) {
	Assert(node_output_channel_ != nullptr);
	frame_shape_ = shp;
	
	std::size_t elem_count = frame_shape_.product();
	ndarray_format frame_format = make_ndarray_format<Output_elem>(elem_count);
	node_output_channel_->define_frame_format(frame_format);
}


template<std::size_t Output_dim, typename Output_elem>
auto filter_output<Output_dim, Output_elem>::frame_shape() const -> const frame_shape_type& {
	Assert(node_output_channel_ != nullptr);
	return frame_shape_;
}


template<std::size_t Output_dim, typename Output_elem>
bool filter_output<Output_dim, Output_elem>::frame_shape_is_defined() const {
	Assert(node_output_channel_ != nullptr);
	return node_output_channel_->frame_format().is_defined();
}


template<std::size_t Output_dim, typename Output_elem>
auto filter_output<Output_dim, Output_elem>::get_output_view
(const frame_view& generic_view) -> view_type {
	std::ptrdiff_t channel_index = index();
	return from_opaque<Output_dim, Output_elem>(extract_part(generic_view, channel_index), frame_shape());
}


////////////////////


template<std::size_t Input_dim, typename Input_elem>
filter_input<Input_dim, Input_elem>::filter_input(filter& filt, time_unit past_window, time_unit future_window) :
	past_window_(past_window),
	future_window_(future_window)
{
	filt.register_input(*this);
}


template<std::size_t Input_dim, typename Input_elem>
void filter_input<Input_dim, Input_elem>::install(processing_node& nd) {
	node_input_ = &nd.add_input();
	node_input_->set_past_window(past_window_);
	node_input_->set_future_window(future_window_);
	
	edge_->set_node_input(*node_input_);
}


template<std::size_t Input_dim, typename Input_elem>
auto filter_input<Input_dim, Input_elem>::frame_shape() const -> const frame_shape_type& {
	return edge_->input_frame_shape();
}

	
template<std::size_t Input_dim, typename Input_elem>
void filter_input<Input_dim, Input_elem>::set_activated(bool act) {
	node_input_->set_activated(act);
}


template<std::size_t Input_dim, typename Input_elem>
bool filter_input<Input_dim, Input_elem>::is_activated() {
	return node_input_->is_activated();
}



template<std::size_t Input_dim, typename Input_elem>
template<std::size_t Output_dim, typename Output_elem>
void filter_input<Input_dim, Input_elem>::connect(filter_output<Output_dim, Output_elem>& out) {
	static_assert(Input_dim == Output_dim, "input and output connected on edge must have same dimension");
	using edge_type = filter_direct_edge<Input_dim, Input_elem, Output_elem>;
	edge_type* edge = new edge_type(*this, out);
	edge_.reset(edge);
	out.edge_has_connected(*edge);
}


template<std::size_t Input_dim, typename Input_elem>
template<std::size_t Output_dim, typename Output_elem, typename Convert_function>
void filter_input<Input_dim, Input_elem>::connect(filter_output<Output_dim, Output_elem>& out, Convert_function&& cv) {
	connect<Output_elem, Input_dim, Output_elem, Convert_function>(out, std::forward<Convert_function>(cv));
}


template<std::size_t Input_dim, typename Input_elem>
template<typename Casted_elem, std::size_t Output_dim, typename Output_elem, typename Convert_function>
void filter_input<Input_dim, Input_elem>::connect(filter_output<Output_dim, Output_elem>& out, Convert_function&& cv) {
	static_assert(Input_dim == Output_dim, "input and output connected on edge must have same dimension");
	using edge_type = filter_converting_edge<Input_dim, Input_elem, Casted_elem, Output_elem, Convert_function>;
	edge_type* edge = new edge_type(*this, out, std::forward<Convert_function>(cv));
	edge_.reset(edge);
	out.edge_has_connected(*edge);
}


template<std::size_t Input_dim, typename Input_elem>
auto filter_input<Input_dim, Input_elem>::get_input_view
(const timed_frame_array_view& generic_view) -> full_view_type {
	// `generic_view` must come from input of (node that is associated to this filter)
	Expects(edge_ != nullptr);
	return edge_->cast_node_output_full_view(generic_view);
}


}}
