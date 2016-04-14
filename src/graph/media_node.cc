#include "media_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf {

void media_node::thread_main_() {
	try{
	for(;;) pull_frame_();
	}catch(int){} 
	MF_DEBUG("node:: ended");
}


void media_node::pull_frame_() {
	MF_DEBUG("node::pull().... (time = ", time_, ")");
	
	time_unit time = -1;
	for(media_node_output_base* output : outputs_) {
		if(! output->is_active()) continue;
		
		time_unit t = output->begin_write();
		if(time == -1) time = t;
		else if(time != t) throw std::runtime_error("wrong time");
	}
	// time = as requested by output (sequential, except when seeked)
	
	
	for(media_node_input_base* input : inputs_) {		
		input->begin_read(time);
	}
		
	time_ = time;
	
	this->process_();
	
	bool reached_end = false;

	MF_DEBUG("node::pull(), before input end_read:\n", inputs_[0]->connected_output());


	for(media_node_input_base* input : inputs_) {
		input->end_read(time_);
		if(input->reached_end()) { reached_end = true; }
	}
	// input reached end (after read) --> this was last frame
	for(media_node_output_base* output : outputs_) {
		output->end_write(reached_end);
	}
	
	MF_DEBUG("node::pull() (t=", time_, ", end=", reached_end, "), \n", *outputs_[0]);

	if(reached_end) throw 1;
}

void media_node::stop_() {
	thread_.join();
}

void media_node::launch_() {
	thread_ = std::move(std::thread((std::bind(&media_node::thread_main_, this))));
}


media_node::media_node(time_unit prefetch) :
	media_node_base(0) { }	

media_node::~media_node() {
	assert(! thread_.joinable());
}

}
