namespace mf { namespace flow {


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::install_() {
	Expects(node_input_ != nullptr && node_output_ != nullptr);
	cast_connector_.reset(new cast_connector_type(*node_output_));
	node_input_->connect(*cast_connector_);
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::set_node_input(node_input& in) {
	Expects(node_input_ == nullptr);
	node_input_ = &in;
	if(node_input_ != nullptr && node_output_ != nullptr) install_();
}


template<std::size_t Input_dim, typename Input_elem, std::size_t Output_dim, typename Output_elem>
void filter_edge<Input_dim, Input_elem, Output_dim, Output_elem>::set_node_output(node_output& out) {
	Expects(node_output_ == nullptr);
	node_output_ = &out;
	if(node_input_ != nullptr && node_output_ != nullptr) install_();
}


}}
