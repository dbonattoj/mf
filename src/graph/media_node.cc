#include "media_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf {

void media_node::thread_main_() {
	for(;;) pull_frame_();
}


void media_node::pull_frame_() {
	time_unit time = -1;
	for(media_node_output_base* output : outputs_) {
		if(! output.is_active()) continue;
		
		time_unit t = output->begin_write();
		if(time == -1) time = t;
		else if(time != t) throw "wrong time";
	}
	for(media_node_input_base* input : inputs_) {
		input->begin_read(time);
	}
	
	this->process_();
	
	for(media_node_input_base* input : inputs_) {
		input->end_read(time_);
	}
	for(media_node_output_base* output : outputs_) {
		output->end_write(false);
	}
}

void media_node::stop_() {

}


media_node::media_node(time_unit prefetch) :
	media_node_base(0),
	thread_(std::bind(&media_node::thread_main_, this)) { }
	

media_node::~media_node() {
	thread_.join();
}

}
