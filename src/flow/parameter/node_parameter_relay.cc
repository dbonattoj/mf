#include "node_parameter_relay.h"

namespace mf { namespace flow {

void node_parameter_relay::set_handler(node_parameter_id id, const send_function_type& func) {
	handlers_.emplace(id, func);
}


bool node_parameter_relay::has_handler(node_parameter_id id) const {
	return (handlers_.find(id) != handlers_.end());
}


auto node_parameter_relay::handler(node_parameter_id id) const -> const handler_function_type& {
	return handlers_.at(id);
}


void node_parameter_relay::send_parameter(node_parameter_id id, const node_parameter_value& new_value) const {
	handler(id)(new_value);
}


}}
