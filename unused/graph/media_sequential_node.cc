#include "media_sequential_node.h"

namespace mf {

void media_sequential_node::pull_frame() {
	// current time = number of frame that is currently processed
	time_++;
	
	// prepare inputs for reading and outputs for writing
	for(auto* input : inputs_) {
		input->pull(); // ...recursively pull required frames from previous nodes
		input->begin_read();
	}
	for(auto* output : outputs_) {
		output->begin_write();
	}
	
	// process in subclass
	// views are now available (input.view(), output.view())
	this->process_();
	
	// end reading and writing
	for(auto* input : inputs_) {
		input->end_read();
	}
	for(auto* output : outputs_) {
		output->end_write(false);
	}
}

}
