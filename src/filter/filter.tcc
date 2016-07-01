#include "filter_connector.h"

namespace mf { namespace flow {

template<std::size_t Dim, typename Elem> template<typename Connector, typename... Args>
void filter::input_port<Dim, Elem>::set_connector(Args&&... args) {
	Connector* connector = new Connector(std::forward<Args>(args)...);
	connector_.reset(connector);
	node_input_.connect(*connector);
}


template<std::size_t Dim, typename Elem>
auto filter::input_port<Dim, Elem>::frame_shape() const -> const frame_shape_type& {
	return connector_->frame_shape();
}

}}
