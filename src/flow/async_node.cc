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
	MF_DEBUG_T("node", name, ": t=", current_time(), " frame...");
	
	// begin writing to outputs
	// outputs determine time of this node
	// seeks gets propagated this way
	time_unit t = -1;
	bool outputs_synchronized = true;
	for(output_base& out : outputs()) {
		MF_DEBUG_T("node", name, " begin_write...");
		time_unit requested_t;
		bool ok = out.begin_write_next_frame(requested_t);
		if(! ok) MF_DEBUG_T("node", name, " begin_write reported end. req_t=", requested_t);
		if(! ok) {
			input_base& in = inputs().front();
			in.begin_read_frame(requested_t-1);
			in.end_read_frame(requested_t-1);
			throw 1;
		}
				
		if(t == -1) t = requested_t;
		else if(requested_t != t) outputs_synchronized = false;
	}
	
	if(! outputs_synchronized) throw std::runtime_error("not sync!");
	
	// don't write more frames than requested
	// async_node::output sets time_limit_ according to current time and prefetch
	// while node is inactive, 
	if(t > time_limit_) {
		MF_DEBUG_T("node", name, " cancel write ", t, " > time limit", time_limit_);
		for(output_base& out : outputs()) out.cancel_write_frame();
		return;
	} else {
		MF_DEBUG_T("node", name, " ok... ", t, " <= time limit", time_limit_);
	}

	if(stream_duration() != -1) MF_ASSERT(t < stream_duration());
	
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
			MF_DEBUG_T("node", name, ": skipping frame ", t, " from input (desactivated)");
			in.skip_frame(t);
		}
	}


	// concrete node processed the frame
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
	
	// end writing to outputs
	for(output_base& out : outputs()) {
		MF_DEBUG_T("node", name, " t=", t, " end_write... (end=", reached_end, ")");
		out.end_write_frame(reached_end);
	}
	
	
	if(reached_end) throw 1;
}


/*

	virtual time_unit begin_write_next_frame() = 0;
	virtual void end_write_frame(bool mark_end = false) = 0;
	virtual void cancel_write_frame() = 0;

	
	
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
*/

}}



/*
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

*/
