#include "source_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf { namespace flow {

void source_node::thread_main_() {
	try{
	for(;;) pull_frame_();
	}catch(int){}
	MF_DEBUG("source:: ended");
}


void source_node::pull_frame_() {
	//if(! is_active()) return;
	
	if(stream_duration() != -1 && time_ == stream_duration()-1) return;
	
	auto outputs = all_outputs();
	
	MF_DEBUG("source::pull().... (t=", time_, ")");
	
	time_unit time = -1;
	for(node_output_base& output : outputs) {
		time_unit t = output.begin_write();
		if(time == -1) time = t;
		else if(time != t) throw std::runtime_error("wrong time");
	}
	
	time_ = time;

	if(stream_duration() != -1)
		assert(time < stream_duration()); // seek must have respected stream duration of this node
	
	this->process();
	
	// check if this was last frame
	
	bool reached_end = false;
	if(stream_duration() != -1) {
		if(time_ == stream_duration() - 1) reached_end = true;
	} else {
		reached_end = this->reached_end();
	}
	
	for(node_output_base& output : outputs) {
		output.end_write(reached_end);
	}
	
	MF_DEBUG("source::pull() (t=", time_, ", end=", reached_end, "), \n", *outputs_[0]);
	
	if(reached_end) throw 1;
}

void source_node::stop() {
	thread_.join();
}

void source_node::launch() {
	thread_ = std::move(std::thread((std::bind(&source_node::thread_main_, this))));
}


source_node::source_node(bool seekable, time_unit stream_duration) {
	define_source_stream_properties(seekable, stream_duration);
}	
	

source_node::~source_node() {
	assert(! thread_.joinable());
}

}}
