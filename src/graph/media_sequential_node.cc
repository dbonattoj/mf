#include "media_sequential_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>

namespace mf {

void media_sequential_node::pull_frame_() {
	if(reached_end_) throw sequencing_error("already reached end");

	// current time = number of frame that is currently processed
	time_++;
		
	// begin reading and writing
	for(media_node_input_base* input : inputs_) {
		// input must not be at end already (then reached_end_ would already be true)
		assert(! input->reached_end()); 
		input->begin_read(time_);
	}
	for(media_node_output_base* output : outputs_)
		output->begin_write();
	
	// process in subclass
	// views are now available (input.view(), output.view())
	this->process_();
	if(this->process_reached_end_()) reached_end_ = true;
	
	// end reading and writing
	for(media_node_input_base* input : inputs_) {
		input->end_read(time_);
		
		// set reached_end_ when input now at end --> no more frames will be available from that input
		if(input->reached_end()) reached_end_ = true;
	}
	for(media_node_output_base* output : outputs_)
		output->end_write(reached_end_);
}


void media_sequential_node::pull(time_unit target_time) {
	while(time_ < target_time && !reached_end_) pull_frame_();
}

}
