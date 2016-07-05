#include "../flow/multiplexer_node.h"

namespace mf { namespace flow {

template<std::size_t Input_dim, typename Input_elem> template<std::size_t Output_dim, typename Output_elem>
void filter_input<Input_dim, Input_elem>::connect(filter_output<Output_dim, Output_elem>& out) {
	using edge_type = filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>;
	edge_type edge = new edge_type();
	edge_.reset(edge);
	out.edge_has_connected(*edge);
}


template<std::size_t Input_dim, typename Input_elem>
void filter_input<Input_dim, Input_elem>::install(filter_node& nd) {
	node_input_ = &nd.add_input();
	node_input_.set_past_window(past_window_);
	node_input_.set_future_window(future_window_);
	
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



///////////////


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::edge_has_connected(edge_base_type& edge) {
	edges_.push_back(&edge);
}


template<std::size_t Output_dim, typename Output_elem>
void filter_output<Output_dim, Output_elem>::install(filter_node& nd) {
	node_output_ = &nd.output();
	node_output_.define_format(frame_format::default_format<Output_elem>(););
	
	if(edges_.size() > 1) {
		multiplex_node_.reset(new multiplex_node);
		multiplex_node_->input().connect(node_output_);
		for(edge_base_type* edge : edges_) {
			multiplex_node_output& mlpx_out = multiplex_node_->add_output();
			edge->set_node_output(mlpx_out);
		}
	} else if(edges_.size() == 1) {
		edges_.front()->set_node_output(*node_output_);
	}
}


void define_frame_shape(const frame_shape_type& shp) {
	frame_shape_ = shp;
	node_output_->define_frame_length(shp.product());
}

const frame_shape_type& frame_shape() const {
	return frame_shape_;
}



}}
