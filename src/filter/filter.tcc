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
#include "../flow/filter_node.h"
#include "../flow/multiplex_node.h"

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
void filter_output<Output_dim, Output_elem>::install(filter_node& nd) {
	node_output_ = &nd.add_output();
	node_output_->define_format(frame_format::default_format<Output_elem>());
	
	if(edges_.size() > 1) {
		multiplex_node_.reset(new multiplex_node(nd.this_graph()));
		multiplex_node_->input().connect(*node_output_);
		for(edge_base_type* edge : edges_) {
			multiplex_node_output& mlpx_out = multiplex_node_->add_output();
			edge->set_node_output(mlpx_out);
		}
	} else if(edges_.size() == 1) {
		edges_.front()->set_node_output(*node_output_);
	}
}


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::define_frame_shape(const frame_shape_type& shp) {
	frame_shape_ = shp;
	node_output_->define_frame_length(shp.product());
}


template<std::size_t Output_dim, typename Output_elem>
auto filter_output<Output_dim, Output_elem>::frame_shape() const -> const frame_shape_type& {
	return frame_shape_;
}


////////////////////


template<std::size_t Input_dim, typename Input_elem>
filter_input<Input_dim, Input_elem>::filter_input(filter& filt, time_unit past_window, time_unit future_window) :
	past_window_(past_window),
	future_window_(future_window)
{
	filt.register_input(*this);
}



template<std::size_t Input_dim, typename Input_elem> template<std::size_t Output_dim, typename Output_elem>
void filter_input<Input_dim, Input_elem>::connect(filter_output<Output_dim, Output_elem>& out) {
	using edge_type = filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>;
	edge_type* edge = new edge_type(*this, out);
	edge_.reset(edge);
	out.edge_has_connected(*edge);
}


template<std::size_t Input_dim, typename Input_elem>
void filter_input<Input_dim, Input_elem>::install(filter_node& nd) {
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

}}
