#include "async_node.h"
#include <functional>

#include <signal.h>

#include <chrono>
using namespace std::chrono;

namespace mf { namespace flow {

async_node::async_node() : node_base() { }


async_node::~async_node() {
	MF_EXPECTS_MSG(!running_, "async_node must have been stopped prior to destruction");
}


async_source_node::async_source_node(bool seekable, time_unit stream_duration) {
	define_source_stream_properties(seekable, stream_duration);
}


bool async_source_node::reached_end() const noexcept {
	MF_ASSERT_MSG(false, "async_source_node::reached_end() must be implemented when stream_duration is not defined.");
	return true;
}


void async_node::stop() {
	MF_EXPECTS(running_);
	MF_EXPECTS(thread_.joinable());
	stop_event_.notify();
	thread_.join();
	running_ = false;
	MF_ENSURES(! running_);
}


void async_node::launch() {
	MF_EXPECTS(! running_);
	thread_ = std::move(std::thread(
		std::bind(&async_node::thread_main_, this)
	));
	running_ = true;
	MF_ENSURES(running_);
}


void async_node::thread_main_() {
	try{
		for(;;) frame_();
	} catch(int){
	
	} 
	MF_DEBUG_T("node", name, " ended");
}


void async_node::frame_() {
	MF_EXPECTS_MSG(outputs().size() == 1, "async_node must have exactly one output");
		
	MF_DEBUG_T("node", name, ": t=", current_time(), " frame...");
	
	//if(! is_active()) { std::this_thread::sleep_for(100ms); return; }
	
	output_base& out = outputs().front();
	//MF_ASSERT(out.is_active());
	
	
	time_unit t;
	bool ok = out.begin_write_next_frame(t);
	if(! ok) throw 1; // at end

	if(stream_duration() != -1) MF_ASSERT(t < stream_duration());
	
	
	
	// don't write more frames than requested
	// async_node::output sets time_limit_ according to current time and prefetch
	if(t >= time_limit_) {
		//MF_DEBUG_T("node", name, " cancel write ", t, " >= time limit", time_limit_);
		//out.cancel_write_frame();
		//return;
	} else {
		MF_DEBUG_T("node", name, " ok... ", t, " < time limit", time_limit_);
	}

	
	// set current time,
	// and allow concrete node to activate/desactivate inputs now
	set_current_time(t);
	this->pre_process();
	
	
	
	// now begin reading from inputs (if any)
	// current time t gets propagated
	for(input_base& in : inputs()) {
		MF_ASSERT_MSG(! in.reached_end(t), "input of node must not already be at end");
		
		if(in.is_activated()) {
			MF_DEBUG_T("node", name, ": reading frame ", t, " from input");
			in.begin_read_frame(t);
		} else {
			MF_DEBUG_T("node", name, ": NOT skipping frame ", t, " from input (desactivated)");
		}
	}

	// concrete node processed the frame
	MF_DEBUG_T("node", name, ": processing frame ", t, " now...");
	this->process();
	
	// check if this node has reached its end
	// the node's end may occur earlier than the end from any of its input
	bool reached_end = false;
	if(stream_duration_is_defined()) {
		if(t == stream_duration() - 1) reached_end = true;
		MF_ASSERT(t < stream_duration());
	} else if(is_source()) {
		reached_end = this->reached_end();
	}

	// end reading from activated inputs
	// also check if any input has now reached its end
	// it this node does not have stream duration, this determines end
	for(input_base& in : inputs()) {
		if(in.is_activated()) {
			MF_DEBUG_T("node", name, ": end_read(", t, ") from input");
			in.end_read_frame(t);
		}
		if(in.reached_end(t)) reached_end = true;
	}
	
	// end writing to output
	MF_DEBUG_T("node", name, " t=", t, " end_write... (end=", reached_end, ")");
	out.end_write_frame(reached_end);

	
	if(reached_end) throw 1;
}

}}

