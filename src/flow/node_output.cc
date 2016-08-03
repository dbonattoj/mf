#include "node_output.h"
#include "node_input.h"

namespace mf { namespace flow {

node_output::node_output(node& nd) :
	node_(nd) { }

void node_output::input_has_connected(node_input& input) {
	connected_input_ = &input;
}


void node_output::input_has_disconnected() {
	connected_input_ = nullptr;
}


bool node_output::is_online() const {
	if(connected_input_->is_activated() == false) return false;
	else return (connected_input_->this_node().state() == node::online);
}


node& node_output::connected_node() const noexcept {
	return connected_input_->this_node();
}


time_unit node_output::end_time() const noexcept {
	return this_node().end_time();
}

}}
