#include "filter_connector.h"

namespace mf { namespace flow {

template<std::size_t Dim, typename Elem> template<std::size_t Output_dim, typename Output_elem>
void filter::input_port<Dim, Elem>::connect(output_port<Output_dim, Output_elem>& output) {
	using connector_type = filter_connector<Dim, Elem, Output_dim, Output_elem>;
	connector_.reset(new connector_type(output));
	node_input_.connect(*connector_);
}


template<std::size_t Dim, typename Elem>
auto filter::input_port<Dim, Elem>::frame_shape() const -> const frame_shape_type& {
	return connector_->frame_shape();
}

}}
