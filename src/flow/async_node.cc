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

#include <iostream>

 #include <signal.h>

namespace mf { namespace flow {
	
async_node::async_node(graph& gr) : filter_node(gr) { }

async_node::~async_node() {
	MF_ASSERT(! running_);
	MF_ASSERT(! thread_.joinable());
}

void async_node::setup() {
	setup_filter();
}

void async_node::launch() {
	MF_ASSERT(! running_);
	thread_ = std::move(std::thread(
		std::bind(&async_node::thread_main_, this)
	));
}

void async_node::pre_stop() {
	MF_RAND_SLEEP;
	MF_DEBUG("ring_->break_reader()");
	ring_->break_reader();
	MF_RAND_SLEEP;
	continuation_cv_.notify_one();
}

void async_node::stop() {
	thread_.join();
}

void async_node::output_setup() {
	const node& connected_node = output().connected_node();
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);

	ndarray_generic_properties prop(output().format(), output().frame_length(), required_capacity);
	ring_.reset(new shared_ring(prop, is_seekable(), stream_duration()));
}

time_unit async_node::minimal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;
	const node_input& in = output().connected_input();
	return in.this_node().minimal_offset_to(target_node) - in.past_window_duration();
}

time_unit async_node::maximal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;
	const node_input& in = output().connected_input();
	return in.this_node().minimal_offset_to(target_node) + in.future_window_duration() + prefetch_duration_;
}

bool async_node::may_continue_() {	
	time_unit next_write_time = ring_->write_start_time();

	MF_DEBUG_EXPR(next_write_time, failed_request_id_, current_request_id_.load(), time_limit_.load(), ring_->end_time());

	if(this_graph().was_stopped()) return true;
	
	if(failed_request_id_ != -1 && current_request_id_ == failed_request_id_) return false;
	if(next_write_time >= time_limit_) return false;
	if(ring_->end_time() != -1 && next_write_time >= ring_->end_time()) return false;
	return true;
}

void async_node::thread_main_() {
	MF_DEBUG("thread");

	
	bool run = true;
	while(run) {
		{
			std::unique_lock<std::mutex> lock(continuation_mutex_);
			continuation_cv_.wait(lock, [&]() { 
				bool c = may_continue_();
				MF_DEBUG_EXPR(c);
				return c;
				
				});
			MF_RAND_SLEEP;
			if(this_graph().was_stopped()) break;
		}
		
		MF_DEBUG("continuation...");
		MF_RAND_SLEEP;
		run = process_frames_();
		
		MF_DEBUG("process_frames_ end. run=", run);
	}
	
	MF_DEBUG("thread end.");
}

bool async_node::process_frames_() {
	// request = launched by output_pull, has (UID, time_limit, start time)
	// frames that are processed after output_pull launched request (pulled span + prefetch) belong to that request
	// failure when processing those frames (got tfail from input pull, crossed time limit, crossed end time, stopped)
	// break output_pull and make it return tfail only when it is waiting for readable frames in same request.
	
	request_id_type request_id;

	// process next frames until paused
	for(;;) {
		MF_DEBUG("process frame...");
		
		// request_id = unique id of current output_pull invocation
		request_id = current_request_id_.load();	

		// wait for frame to become writable, and get view to it
		// defines the time of frame to process, output_pull can seek to new time
		// output_pull sets buffer time (seek) and time_limit before request_id.
		// if now out_vw.start_time() is already set, but request_id still has old value and failure occurs,
		// goes to pause. output_pull then notifies continuation_cv_ after setting right request_id, and then the
		// request gets processed.
		
		MF_RAND_SLEEP;
		auto out_vw = ring_->try_begin_write(1);
		if(out_vw.is_null()) break;
		if(out_vw.duration() == 0) break;

		time_unit request_time = out_vw.start_time();
		MF_DEBUG("process frame... request_time=", request_time, " limit=", time_limit_);

		MF_RAND_SLEEP;
		if(request_time >= time_limit_) {
			ring_->end_write(0);
			break;
		}
		
		time_unit t = out_vw.start_time();
		node_job job = make_job();
		job.define_time(t);
		MF_RAND_SLEEP;
		set_current_time(t);
		
		MF_RAND_SLEEP;
		if(state() == reconnecting && reconnect_flag_) {
			set_online();
			reconnect_flag_ = false;
		}
		
		pre_process_filter(job);
		
		job.push_output(output(), out_vw[0]);
		
		for(auto&& in : inputs()) if(in->is_activated()) {
			time_unit res = in->pull(t);
			if(res == pull_result::stopped) { ring_->end_write(0); job.pop_output(); goto fail; }
			else if(res == pull_result::temporary_failure) { ring_->end_write(0); job.pop_output(); goto fail; }
		}
		
		for(auto&& in : inputs()) if(in->is_activated()) {
			timed_frame_array_view in_view = in->begin_read_frame(t);
			if(in_view.is_null()) { ring_->end_write(0); job.pop_output(); return false; }
			MF_ASSERT(in_view.span().includes(t));
			
			job.push_input(*in, in_view);
		}
		
		MF_DEBUG("process ", t);
		process_filter(job);
		
		bool reached_end = false;
		if(stream_duration_is_defined()) {
			if(t == stream_duration() - 1) reached_end = true;
			MF_ASSERT(t < stream_duration());
		} else if(is_source()) {
			reached_end = job.end_was_marked();
		}
	
		while(node_input* in = job.pop_input()) {
			in->end_read_frame(t);
			if(t == in->end_time() - 1) reached_end = true;
		}
				
		if(reached_end) {
			bool mark_last_frame = !ring_->is_seekable();
			ring_->end_write(1, mark_last_frame);			
			mark_end();
		} else {
			ring_->end_write(1);
		}
		
		job.pop_output();
		
		if(reached_end) break;
	}

fail:
	
	// mark request_id as failed, and break output_pull waiting for readable frame
	failed_request_id_ = request_id;
	MF_RAND_SLEEP;
	ring_->break_reader();

	return true;
}

node::pull_result async_node::pull_(time_span& pull_span, bool reconnect) {
	MF_DEBUG("output: pull ", pull_span);
	{
		std::lock_guard<std::mutex> lock(continuation_mutex_);
		time_limit_.store(pull_span.end_time() + prefetch_duration_ + 1);
		
		time_unit pull_time = pull_span.start_time();
		if(pull_time >= ring_->read_start_time()) ring_->skip(pull_time - ring_->read_start_time());
		else ring_->seek(pull_span.start_time());
		
		if(reconnect) reconnect_flag_ = true;
		
		current_request_id_++;
	}
	
	MF_RAND_SLEEP;
	continuation_cv_.notify_one();
	
	while(ring_->readable_duration() < pull_span.duration()) {
		MF_RAND_SLEEP;		
		MF_ASSERT(ring_->read_start_time() == pull_span.start_time());
		
		MF_DEBUG("output: pull ", pull_span, " : wait_readable...");

if(this_graph().was_stopped()) return pull_result::stopped;
		bool became_readable = ring_->wait_readable();
		MF_DEBUG("output: pull ", pull_span, " : wait_readable. readable=", ring_->readable_duration());
		
		if(! became_readable) {
			bool this_request_failed = (failed_request_id_ == current_request_id_);
			if(this_graph().was_stopped()) {
				MF_DEBUG("output: pull -> stopped");
				return pull_result::stopped;
			} else if(this_request_failed) {
				MF_DEBUG("output: pull -> tfail");
				return pull_result::temporary_failure;
			}
		} else if(became_readable && ring_->writer_reached_end()) {
			pull_span = time_span(pull_span.start_time(), pull_span.start_time() + ring_->readable_duration());
		}
		
	}
	
	MF_DEBUG("output: pull -> success");
	
	return pull_result::success;
}

timed_frame_array_view async_node::begin_read_(time_unit duration)  {
	return ring_->try_begin_read(duration);
}

void async_node::end_read_(time_unit duration) {
	ring_->end_read(duration);
}


async_node& async_node_output::this_node() {
	return static_cast<async_node&>(node_output::this_node());
}

const async_node& async_node_output::this_node() const {
	return static_cast<const async_node&>(node_output::this_node());
}

void async_node_output::setup() {
	this_node().output_setup();
}

node::pull_result async_node_output::pull(time_span& span, bool reconnect) {
	return this_node().pull_(span, reconnect);
}

timed_frame_array_view async_node_output::begin_read(time_unit duration) {
	return this_node().begin_read_(duration);
}

void async_node_output::end_read(time_unit duration) {
	return this_node().end_read_(duration);
}

time_unit async_node_output::end_time() const {
	return this_node().end_time();
}


async_node_output& async_node::output() { return static_cast<async_node_output&>(*outputs().front()); }
const async_node_output& async_node::output() const { return static_cast<const async_node_output&>(*outputs().front()); }



node_output& async_node::add_output(const frame_format& format) {
	return add_output_<async_node_output>(format);
}




}}
