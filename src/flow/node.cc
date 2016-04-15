#include "node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf { namespace flow {

void node::thread_main_() {
	try{
	for(;;) pull_frame_();
	}catch(int){} 
	MF_DEBUG("node:: ended");
}


void node::pull_frame_() {
//	if(! is_active()) return;
		
	auto outputs = all_outputs();
	
	MF_DEBUG("node::pull().... (time = ", time_, ")");
		
	time_unit time = -1;
	for(node_output_base& output : outputs) {		
		time_unit t = output.begin_write();
		if(time == -1) time = t;
		else if(time != t) throw std::runtime_error("wrong time");
	}

	// time = as requested by output (sequential, except when seeked)

	time_ = time;

	this->pre_process();

	
	for(node_input_base* input : inputs_) {
		assert(! input->reached_end());
		
		if(input->is_activated())
			input->begin_read(time_);
		else
			input->skip(time_);
	}
		
	
	this->process();
	
	bool reached_end = false;

	MF_DEBUG("node::pull(), before input end_read:\n", inputs_[0]->connected_output());


	for(node_input_base* input : inputs_) {
		if(input->is_activated())
			input->end_read(time_);
			
		if(input->reached_end()) reached_end = true;
	}
	
	for(node_output_base& output : outputs) {
		output.end_write(reached_end);
	}
	
	MF_DEBUG("node::pull() (t=", time_, ", end=", reached_end, "), \n", *outputs_[0]);

	if(reached_end) throw 1;
}
	


void node::stop() {
	thread_.join();
}

void node::launch() {
	thread_ = std::move(std::thread((std::bind(&node::thread_main_, this))));
}


node::node(time_unit prefetch) :
	node_base() { }	

node::~node() {
	assert(! thread_.joinable());
}

}}
