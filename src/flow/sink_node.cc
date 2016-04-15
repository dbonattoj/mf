#include "sink_node.h"

namespace mf { namespace flow {

void sink_node::pull_frame_() {
	MF_DEBUG("sink::pull().... (t=", time_, ")");
	
	// sink controlls time flow --> propagated to rest of graph
	
	time_++;

	for(node_input_base* input : inputs_) {
		input->begin_read(time_);
	}
		
	this->process_();
	
	for(node_input_base* input : inputs_) {
		input->end_read(time_);
		if(input->reached_end()) reached_end_ = true;
	}
	
	if(reached_end_) MF_DEBUG("sink: reached end!");

	MF_DEBUG("sink::pull() (t=", time_, ")");
}


sink_node::sink_node() :
	node_base(0) { }	


void sink_node::setup_graph() {
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_setup_();
}

void sink_node::stop_graph() {
}

void sink_node::seek(time_unit t) {
	if(is_seekable()) time_ = t - 1;
	else throw std::logic_error("sink node is not seekable");
}



void sink_node::pull_next_frame() {
	pull_frame_();
}

}}
