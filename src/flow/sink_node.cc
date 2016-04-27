#include "sink_node.h"
#include <stdexcept>

namespace mf { namespace flow {

void sink_node::frame_() {	
	MF_EXPECTS_MSG(! reached_end_, "end must not already have been reached");

	time_unit t = current_time();
		
	// the sink node controls time
	t++;
	set_current_time(t);
		
	if(stream_duration_is_defined())
		MF_ASSERT_MSG(t < stream_duration(), "sink must not already be at end");
	
	// allow concrete node to activate/desactivate inputs
	this->pre_process();
	
	// begin reading from activated inputs,
	// skip frame in desactivated inputs
	for(input_base& in : inputs()) {
		MF_ASSERT_MSG(! in.reached_end(t), "input of sink must not already be at end");
		
		if(in.is_activated()) {
			MF_DEBUG_T("node", name, ": reading frame ", t, " from input");	
			bool continuing = in.begin_read_frame(t);
			if(! continuing) {
				MF_DEBUG_T("node", "stop from input");
				return;
			}
		}
	}
	
	// concrete node processes frame
	this->process();
	
	// end reading from activated inputs
	// sink has reached end if any input (including desactivated) reached end
	for(input_base& in : inputs()) {
		if(in.is_activated()) {
			in.end_read_frame(t);
		}
		
		if(in.reached_end(t)) reached_end_ = true;
	}
	
	// in case all inputs were deactivated, test against stream duration for end
	if(stream_duration_is_defined())
		if(t == stream_duration() - 1) reached_end_ = true;
		
	// TODO: if no active inputs, don't allow crossing end
}


void sink_node::setup_graph() {
	setup_sink();
}


void sink_node::pull_next_frame() {
	if(reached_end_) throw std::logic_error("sink is already at end");
	frame_();
}

void sink_node::seek(time_unit t) {
	if(! is_seekable()) throw std::logic_error("sink if not seekable");
	if(t < 0 || t >= stream_duration()) throw std::out_of_range("seek out of bounds");
	set_current_time(t - 1);
	reached_end_ = false;
}

}}
