#include "sink_node.h"

namespace mf { namespace flow {

void sink_node::pull_frame_() {

	MF_DEBUG("sink::pull().... (t=", time_, ")");
	
	// sink controlls time flow --> propagated to rest of graph
	
	time_++;
	
	if(time_ == stream_duration_) { reached_end_ = true; return; }

	for(node_input_base* input : inputs_) {
		assert(! input->reached_end());
		
		if(input->is_activated())
			input->begin_read(time_);
		else
			input->skip(time_);
	}
		
	this->process();
	
	for(node_input_base* input : inputs_) {
		if(input->is_activated())
			input->end_read(time_);
			
		if(input->reached_end()) reached_end_ = true;
	}
	
	if(reached_end_) MF_DEBUG("sink: reached end!");

	MF_DEBUG("sink::pull() (t=", time_, ")");
}


sink_node::sink_node() :
	node_base() { }	


void sink_node::setup_graph() {
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_setup_();
}

void sink_node::stop_graph() {
}

void sink_node::seek(time_unit t) {
	reached_end_ = false;
	if(is_seekable()) time_ = t - 1;
	else throw std::logic_error("sink node is not seekable");
}


void sink_node::pull_next_frame() {
	pull_frame_();
}

}}
