#include "filter_connector.h"

namespace mf { namespace flow {


template<std::size_t Dim, typename Elem>
filter::output_port<Dim, Elem>::output_port(filter& filt) :
	base(filt),
	node_output_(filt.this_node().add_output())
{
	node_output_.define_format(base::default_format());
}

template<std::size_t Dim, typename Elem>
filter::input_port<Dim, Elem>::input_port(filter& filt, time_unit past_window, time_unit future_window) :
	base(filt),
	node_input_(filt.this_node().add_input())
{
	node_input_.set_past_window(past_window);
	node_input_.set_future_window(future_window);
}


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
