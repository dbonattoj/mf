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
	//if(! is_active()) return;
		
	//std::this_thread::sleep_for(400ms);

	if(stream_duration() != -1 && time_ == stream_duration()-1) throw 1;

	
	auto outputs = all_outputs();
	
	MF_DEBUG("source::pull().... (t=", time_, ")");
	
	time_unit time = -1;
	for(node_output_base& output : outputs) {
		time_unit t = output.begin_write();
		if(t == -1) { time_ = stream_duration()-1; return; }
		
		if(time == -1) time = t;
		else if(time != t) throw std::runtime_error("wrong time");
	}
		
	MF_DEBUG("source:buffer time=", time);
	
	
	if(time > time_limit_) {
		MF_DEBUG("source::nowrite, ", time_, " > ", time_limit_);
		for(node_output_base& output : outputs) {
			output.didnt_write();
		}
		MF_DEBUG("source::nowrite DONE, ", time_, " > ", time_limit_);
		
		return;
	}

	time_ = time;
	
	if(stream_duration() != -1)
		assert(time < stream_duration()); // seek must have respected stream duration of this node
	
	this->pre_process();
	
	for(node_input_base* input : inputs_) {
		assert(! input->reached_end());
		
		if(input->is_activated())
			input->begin_read(time_);
		else
			input->skip(time_);
	}
	
	MF_DEBUG("processing t=", time_, ", output buffer: ", *outputs_[0]);
	this->process();
	
	// check if this was last frame
	
	bool reached_end = false;
	if(stream_duration() != -1) {
		if(time_ == stream_duration() - 1) reached_end = true;
	}
	
	for(node_input_base* input : inputs_) {
		if(input->is_activated())
			input->end_read(time_);
			
		if(input->reached_end()) reached_end = true;
	}

	
	for(node_output_base& output : outputs) {
		output.end_write(reached_end);
	}
	
	MF_DEBUG("source::pull() (t=", time_, ", end=", reached_end, "), \n", *outputs_[0]);
	
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
