#include "media_source_node.h"

#include <cassert>
#include <typeinfo>
#include <algorithm>
#include <exception>

namespace mf {

void media_source_node::thread_main_() {
	try{
	for(;;) pull_frame_();
	}catch(int){}
	MF_DEBUG("source:: ended");
}


void media_source_node::pull_frame_() {
	MF_DEBUG("source::pull().... (t=", time_, ")");
	
	time_unit time = -1;
	for(media_node_output_base* output : outputs_) {
		if(! output->is_active()) continue;
		
		time_unit t = output->begin_write();
		if(time == -1) time = t;
		else if(time != t) throw std::runtime_error("wrong time");
	}
	
	time_ = time;

	if(stream_duration_is_defined())
		assert(time < stream_duration()); // seek must have respected stream duration of this node
	
	this->process_();
	
	// check if this was last frame
	
	bool reached_end = false;
	if(stream_duration_is_defined()) {
		if(time_ == stream_duration() - 1) reached_end = true;
	} else {
		reached_end = this->reached_end();
	}
	
	for(media_node_output_base* output : outputs_) {
		output->end_write(reached_end);
	}
	
	MF_DEBUG("source::pull() (t=", time_, ", end=", reached_end, "), \n", *outputs_[0]);
	
	if(reached_end) throw 1;
}

void media_source_node::stop_() {
	thread_.join();
}

void media_source_node::launch_() {
	MF_DEBUG("source:: launch");
	thread_ = std::move(std::thread((std::bind(&media_source_node::thread_main_, this))));
}


media_source_node::media_source_node(bool seekable, time_unit stream_duration) :
	media_node_base(0) { if(seekable) define_stream_duration(stream_duration); }	
	

media_source_node::~media_source_node() {
	assert(! thread_.joinable());
}

}
