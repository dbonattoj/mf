/*
#include "thin_node.h"

namespace mf { namespace flow {

void thin_node::internal_setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("thin_node must have exactly 1 output");
	if(inputs().size() != 1) throw invalid_flow_graph("thin_node must have exactly 1 input");
	if(inputs().past_window_duration() != 0) throw invalid_flow_graph("thin_node input must have no past window");
	if(inputs().future_window_duration() != 0) throw invalid_flow_graph("thin_node input must have no futurer window");
	this->setup();	
}


void thin_node::launch() { }

void thin_node::stop() { }



void thin_node_output::pull(time_span span) {
}


timed_frames_view thin_node_output::begin_read(time_unit duration) {

}


void thin_node_output::end_read(time_unit duration) {
	
}


time_unit thin_node_output::end_time() const {
	
}


frame_view thin_node_output::begin_write_frame(time_unit& t) {
	//
}


void thin_node_output::end_write_frame(bool was_last_frame) {
	//
}


void thin_node_output::cancel_write_frame() {
	//
}

}}
*/
