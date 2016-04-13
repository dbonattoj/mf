#include "media_sink_node.h"

namespace mf {

void media_sink_node::pull_frame_() {
	MF_DEBUG("sink::pull().... (t=", time_, ")");
	
	// sink controlls time flow --> propagated to rest of graph
	
	time_++;

	for(media_node_input_base* input : inputs_) {
		input->begin_read(time_);
	}
		
	this->process_();
	
	for(media_node_input_base* input : inputs_) {
		input->end_read(time_);
		if(input->reached_end()) reached_end_ = true;
	}
	
	if(reached_end_) MF_DEBUG("sink: reached end!");

	MF_DEBUG("sink::pull() (t=", time_, ")");
}


void media_sink_node::stop_() {
}

void media_sink_node::launch_() {
}


media_sink_node::media_sink_node() :
	media_node_base(0) { }	


void media_sink_node::setup_graph() {
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_stream_durations_();
	propagate_setup_();
}

void media_sink_node::stop_graph() {
}

void media_sink_node::seek(time_unit t) {
	time_ = t - 1;
}



void media_sink_node::pull_next_frame() {
	pull_frame_();
}



}
