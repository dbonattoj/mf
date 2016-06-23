/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "async_node.h"
#include "node_job.h"
#include "graph.h"
#include <unistd.h>
#include <cstdlib>

#include <iostream>

namespace mf { namespace flow {
	
frame_view async_node_output::begin_write_frame(time_unit& t) {
	async_node& nd = (async_node&)(this_node());

	for(;;) {
		// wait until 1+ frame is writable, or stopped
		// keep waiting if at end
		bool got_writable = ring_->wait_writable();
		if(! got_writable) return frame_view::null();

		// A
		// try begin write
		// null if frame now no longer writable (seeked in A),
		// or duration = 0 (seeked to end in A)
		timed_frame_array_view view = ring_->try_begin_write(1);
		if(view.is_null() || view.duration() != 1) continue;
		
		MF_ASSERT(view.duration() == 1);
		t = view.start_time();
		
		MF_DEBUG("time_lim = ", time_limit_.load());

		
		if(t < time_limit_) {
			return view[0];
		} else {
			MF_DEBUG("-------------------> ", t, " >= timelim ", time_limit_);
			ring_->end_write(0);
			
			nd.failed_time_limit_ = time_limit_;
			
			return frame_view::null();
		}
	}
}	


bool async_node::process_next_frame() {
	//usleep(std::rand() % 200000);
	//usleep(100000);

	node_job job = make_job();
	time_unit t;
		
	// begin writing frame to output
	// output determines time t of frame to write
	auto&& out = outputs().front();
	auto out_view = out->begin_write_frame(t);
	if(out_view.is_null()) return false; // stopped or paused
	
//MF_DEBUG("async: ", t);
	
	set_current_time(t);
	job.define_time(t);
	
	if(state() == reconnecting && reconnect_flag_) {
		set_online();
		reconnect_flag_ = false;
	}

	// with time defined, run preprocess
	// concrete node may activate/desactivate inputs now
	pre_process_filter(job);

	// add output view to job
	job.push_output(*out, out_view);
	
	// pull & begin reading from activated inputs
	for(auto&& in : inputs()) if(in->is_activated()) {
		// pull frame t from input
		// for sequential node: frame is now processed
		time_unit pull_result = in->pull(t);
		if(pull_result == node::pull_stopped) {
			return false;
		} else if(pull_result == node::pull_temporary_failure) {
			return false;
		}
	}
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		// begin reading frame 
		timed_frame_array_view in_view = in->begin_read_frame(t);
		MF_ASSERT(! in_view.is_null());
		MF_ASSERT(in_view.span().includes(t));
		
		// add to job
		job.push_input(*in, in_view);
	}
	
	// process frame in concrete subclass
	process_filter(job);
	
	// check if node reached end
	// determined by stream duration or reached_end() for source
	// when stream duration undefined, end is reached when an input reached it
	bool reached_end = false;
	if(stream_duration_is_defined()) {
		if(t == stream_duration() - 1) reached_end = true;
		MF_ASSERT(t < stream_duration());
	} else if(is_source()) {
		reached_end = job.end_was_marked();
	}

	// end reading from inputs
	while(node_input* in = job.pop_input()) {
		in->end_read_frame(t);
		if(t == in->end_time() - 1) reached_end = true;
	}
	
	// end writing to output,
	// indicate if last frame was reached
	job.pop_output()->end_write_frame(reached_end);
	
	if(reached_end) mark_end();
	
	//MF_DEBUG("processed ", t);
		
	return true;
}


void async_node::thread_main_() {
	async_node_output& out = (async_node_output&)*(outputs().front());

	for(;;) {
		std::unique_lock<std::mutex> lock(active_mutex_);	
		while(!stop_ && !active_) {
			MF_DEBUG("waiting...");
			active_cv_.wait(lock);
			MF_DEBUG("waited. stop=", stop_.load(), " active=", active_.load());
		}	
		lock.unlock();
		
		if(stop_) break;
		
		MF_DEBUG("process next...");
		while(process_next_frame());
		
		lock.lock();		
		if(failed_time_limit_ == out.time_limit_) {
			active_.store(false);
			out.ring_->break_waiting();
		}
		lock.unlock();
	}
	
	MF_DEBUG("async thread end");
}


async_node::async_node(graph& gr) : filter_node(gr) {
	set_prefetch_duration(0);
}


async_node::~async_node() {
	MF_EXPECTS_MSG(!running_, "async_node must have been stopped prior to destruction");
}


void async_node::setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("async_node must have exactly 1 output");
	setup_filter();
}
	

void async_node::launch() {
	MF_EXPECTS(! running_);
	thread_ = std::move(std::thread(
		std::bind(&async_node::thread_main_, this)
	));
	running_ = true;
}


void async_node::pre_stop() {
	async_node_output& out = (async_node_output&)*(outputs().front());
	
	MF_DEBUG("pre_stop...");
	stop_ = true;
	MF_DEBUG("pre_stop: break_waiting");
	out.ring_->break_waiting();
	MF_DEBUG("pre_stop: active_cv_ notify");
	active_cv_.notify_one();
}


void async_node::stop() {
	MF_DEBUG("stop...");
	MF_EXPECTS(running_);
	MF_EXPECTS(thread_.joinable());
	MF_DEBUG("stop: thread join");
	thread_.join();
	running_ = false;
	MF_DEBUG("stop.");
}


void async_node_output::setup() {
	node& connected_node = connected_input().this_node();
	
	time_unit required_capacity = 1 + this_node().maximal_offset_to(connected_node) - this_node().minimal_offset_to(connected_node);

	ndarray_generic_properties prop(format(), frame_length(), required_capacity);
	ring_.reset(new shared_ring(prop, this_node().is_seekable(), this_node().stream_duration()));
}	


time_unit async_node_output::pull(time_span span, bool reconnected) {
	MF_DEBUG("async pull... span=", span);

	async_node& nd = (async_node&)(this_node());

	{
		std::lock_guard<std::mutex> lock(nd.active_mutex_);
		time_limit_ = span.end_time() + this_node().prefetch_duration();
		MF_DEBUG("async_pull: set time_lim = ", time_limit_.load());
	}
	
	MF_DEBUG("async pull: skip/seek");
	time_unit t = span.start_time();
	time_unit ring_read_t = ring_->read_start_time();
	if(t != ring_read_t) {
		MF_DEBUG("async pull: seek: ", t, " != ", ring_read_t);
		if(ring_->is_seekable()) ring_->seek(t);
		else if(t > ring_read_t) ring_->skip(t - ring_read_t);
		else throw std::logic_error("ring not seekable but async_node output attempted to seek to past");
	}
	
	if(reconnected) {
		async_node& nd = (async_node&)this_node();
		nd.reconnect_flag_ = true;
	}
	
	
	MF_DEBUG("async pull: active notify");
	{
		std::lock_guard<std::mutex> lock(nd.active_mutex_);
		nd.active_ = true;
	}
	nd.active_cv_.notify_one();

	
	MF_DEBUG("async pull: wait readable");
	while(ring_->readable_duration() < span.duration()) {
		bool cont = ring_->wait_readable();
		if(cont) {
			if(ring_->writer_reached_end()) return ring_->readable_duration();
		} else {
			if(this_node().this_graph().was_stopped()) return node::pull_stopped;
			else return node::pull_temporary_failure; // only if it won't ever get readable (before relaunch)
		}
	}
	
	MF_DEBUG("async pull. read_start_t=", ring_->read_start_time());
	return ring_->readable_duration();
}


timed_frame_array_view async_node_output::begin_read(time_unit duration) {
	auto vw = ring_->try_begin_read(duration);
	MF_DEBUG("async begin_read. read_start_t=", vw.start_time());

	MF_ASSERT(! vw.is_null());
	return vw;
}


void async_node_output::end_read(time_unit duration) {
	ring_->end_read(duration);
}


time_unit async_node_output::end_time() const {
	return ring_->end_time();
}


void async_node_output::end_write_frame(bool was_last_frame) {
	bool mark_end = was_last_frame && !ring_->is_seekable();
	ring_->end_write(1, mark_end);
}


void async_node_output::cancel_write_frame() {
	ring_->end_write(0, false);
}

}}
