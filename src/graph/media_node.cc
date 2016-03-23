#include "media_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf {

void media_node::pull_frame_() {	
	if(reached_end_) throw sequencing_error("already reached end");
		
	// begin reading and writing
	for(media_node_input_base* input : inputs_) {
		// input must not be at end already (then reached_end_ would already be true)
		assert(! input->reached_end());
		input->begin_read(time_ + 1);
	}
	for(media_node_output_base* output : outputs_) {
		output->begin_write();
	}
	
	// current time = number of frame that is currently processed
	time_++;
	
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
	for(media_node_output_base* output : outputs_) {
		output->end_write(reached_end_);
	}
}


void media_node::thread_main_() {
	while(! reached_end_)
		pull_frame_();
}


void media_node::pull(time_unit target_time) {
	if(sequential_) {
		// sequential mode: process the required frame(s) now
		while(time_ < target_time && !reached_end_) pull_frame_();
	} else {
		// parallel mode: background thread will process the frame(s)
		// launch it if not done yet
		if(! thread_.joinable())
			thread_ = std::thread(std::bind(&media_node::thread_main_, this));
	}
}


void media_node::stop_() {
	if(sequential_) return;

	if(thread_.joinable()) {
		thread_.join();
	}
}


media_node::media_node(time_unit prefetch) :
	media_node_base(prefetch), sequential_(prefetch == 0) { }
	

media_node::~media_node() {
	assert(! thread_.joinable());
}

}
