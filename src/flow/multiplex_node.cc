#include "multiplex_node.h"

namespace mf { namespace flow {

multiplex_node::multiplex_node(graph& gr) : node(gr) {
	
}


multiplex_node::~multiplex_node() {
	
}


time_unit multiplex_node::minimal_offset_to(const node& target_node) const {
	
}


time_unit multiplex_node::maximal_offset_to(const node& target_node) const {
	
}

	
void multiplex_node::launch() {
	
}


void multiplex_node::stop() {
	
}


void multiplex_node::pre_setup() {
	
}

	
node_input& multiplex_node::input() {
	Assert(inputs().size() == 1);
	return *inputs().front();
}


multiplex_node_output& multiplex_node::add_output() {
	return node::add_output_<multiplex_node_output>(format);
}


node::pull_result multiplex_node_output::pull(time_span& span, bool reconnect) {

}


timed_frame_array_view multiplex_node_output::begin_read(time_unit duration) {
	
}


void multiplex_node_output::end_read(time_unit duration) {
	
}



}}
