#include "media_node.h"

#include <algorithm>
#include "media_node_input.h"
#include "media_node_output.h"

namespace mf {

void media_node::register_input_(media_node_input_base& input) {
	inputs_.push_back(&input);
}


void media_node::register_output_(media_node_output_base& output) {
	outputs_.push_back(&output);
}


bool media_node::process_reached_end_() {
	return false;
}


void media_node::propagate_offset_(time_unit new_offset) {
	if(new_offset <= offset_) return;
	
	offset_ = new_offset;
	
	for(media_node_input_base* input : inputs_) {
		media_node& connected_node = input->connected_output().node();
		time_unit off = offset_ + connected_node.prefetch_duration_ + input->future_window_duration();
		connected_node.propagate_offset_(off);
	}		
}


void media_node::propagate_output_buffer_durations_() {
	for(media_node_input_base* input : inputs_) {
		auto& output = input->connected_output();
		media_node& connected_node = output.node();
		time_unit dur = 1 + input->past_window_duration() + (connected_node.offset_ - offset_);
		output.define_required_buffer_duration(dur);
		connected_node.propagate_output_buffer_durations_();
	}
}


void media_node::propagate_setup_() {
	// do nothing when did_setup_ is already set:
	// during recursive propagation it may be called multiple times on same node
	if(did_setup_) return;
	
	// first set up preceding nodes
	for(media_node_input_base* input : inputs_) {
		media_node& connected_node = input->connected_output().node();
		connected_node.propagate_setup_();
	}
	
	// set up this node in concrete subclass
	this->setup_();
	
	// set up outputs
	// their frame shape are now defined, and required durations were defined before
	// (in propagate_output_buffer_durations_())
	for(media_node_output_base* output : outputs_)
		output->setup();
	
	did_setup_ = true;
}


}