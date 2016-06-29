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
#include "graph.h"

namespace mf { namespace flow {
	
async_node::async_node(graph& gr) : filter_node(gr) { }

async_node::~async_node() {
	Assert(! running_);
	Assert(! thread_.joinable());
}

void async_node::setup() {
	setup_filter_();
	
	const node& connected_node = output().connected_node();
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);

	ndarray_generic_properties prop(output().format(), output().frame_length(), required_capacity);
	ring_.reset(new shared_ring(prop, stream_properties().is_seekable(), stream_properties().duration()));
}

void async_node::launch() {
	Assert(! running_);
	thread_ = std::move(std::thread(
		std::bind(&async_node::thread_main_, this)
	));
	running_ = true;
}

void async_node::pre_stop() {
	Expects(running_);
	MF_RAND_SLEEP;
	MF_DEBUG("ring_->break_reader()");
	ring_->break_reader();
	MF_RAND_SLEEP;
	continuation_cv_.notify_one();
}

void async_node::stop() {
	Expects(running_);
	thread_.join();
	running_ = false;
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


bool async_node::pause_() {
	// pause as long as either:
	// - next write time (determined by shared_ring, and reader may seek) is beyond time limit (set by reader)
	// - next write time is beyond end of stream (marked in shared_ring)
	// - received temporary failure from input and reader has not made new request since
	// not pause when graph stopped and return false instead then
	// conditions are rechecked when continuation_cv_ is notified
	// = when reader pulls new frame (after setting request_id)
	//   or when graph is stopped
	
	auto may_continue = [&]()->bool {
		time_unit next_write_time = ring_->write_start_time();
		MF_DEBUG_EXPR(next_write_time, failed_request_id_, current_request_id_.load(), time_limit_.load(), ring_->end_time(), this_graph().was_stopped());
		if(this_graph().was_stopped()) return true;
		else if(failed_request_id_ != -1 && current_request_id_ == failed_request_id_) return false;
		else if(next_write_time >= time_limit_) return false;
		else if(ring_->end_time() != -1 && next_write_time >= ring_->end_time()) return false;
		else return true;
	};

	MF_DEBUG("pause..locking");
	std::unique_lock<std::mutex> lock(continuation_mutex_);
	MF_DEBUG("pause..waiting");
	continuation_cv_.wait(lock, may_continue);
	return ! this_graph().was_stopped();
}


void async_node::thread_main_() {
	bool pause = false;
	
	if(inputs().size() == 1)
		MF_DEBUG("thread (input: -", inputs().front()->past_window_duration(), ", +", inputs().front()->future_window_duration(), ")");
	else
		MF_DEBUG("thread (source)");
		
	for(;;) {
		if(pause) {
			MF_DEBUG("pause...");
			bool cont = pause_();
			if(! cont) break;
			pause = false;
		}
		
		MF_DEBUG("continuation...");
		
		request_id_type request_id = current_request_id_.load();	
		process_result result = process_frame_();
		if(result == process_result::failure) {
			failed_request_id_ = request_id;
			ring_->break_reader();
			pause = true;
		} else if(result == process_result::should_pause) {
			pause = true;
		}
	}
	MF_DEBUG("thread end.");
}


async_node::process_result async_node::process_frame_() {
	MF_RAND_SLEEP;
	auto out_vw = ring_->try_begin_write(1);
	if(out_vw.is_null()) { MF_DEBUG("process: out_vw=null  --> should_pause"); return process_result::should_pause; }
	if(out_vw.duration() == 0) { MF_DEBUG("process: out_vw=()  --> should_pause"); return process_result::should_pause; }

	time_unit request_time = out_vw.start_time();
	MF_DEBUG("process frame... request_time=", request_time, " limit=", time_limit_);

	MF_RAND_SLEEP;
	if(request_time >= time_limit_) {
		ring_->end_write(0);
		MF_DEBUG("process: t=", request_time, " > limit ", time_limit_.load(), " --> failure"); 
		return process_result::failure;
	}
	
	time_unit t = out_vw.start_time();

	set_current_time_(t);
	filter_node_job job = begin_job_();

	job.attach_output(out_vw[0]);
	MF_RAND_SLEEP;
	
	MF_RAND_SLEEP;
	if(state() == reconnecting && reconnect_flag_) {
		set_online();
		reconnect_flag_ = false;
	}
	
	pre_process_filter_(job);
			
	for(auto&& in : inputs()) if(in->is_activated()) {
		time_unit res = in->pull();
		if(res == pull_result::stopped || res == pull_result::temporary_failure) {
			job.detach_output();
			ring_->end_write(0);
			MF_DEBUG("process frame... t=", request_time, " in fail --> failure");
			return process_result::failure;
		}
	}
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		bool cont = job.push_input(*in);
		if(! cont) {
			job.detach_output();
			ring_->end_write(0);
			MF_DEBUG("process frame... t=", request_time, " in broke --> failure");
			return process_result::failure;
		}
	}
	
	MF_DEBUG("process ", t);
	process_filter_(job);

	finish_job_(job);
	
	job.detach_output();
	
	if(reached_end()) {
		bool mark_last_frame = !ring_->is_seekable();
		ring_->end_write(1, mark_last_frame);
		MF_DEBUG("process frame... t=", request_time, " --> should_pause");	
		return process_result::should_pause;
	} else {
		ring_->end_write(1);
		MF_DEBUG("process frame... t=", request_time, " --> should_continue");	
		return process_result::should_continue;
	}
}


node::pull_result async_node::output_pull_(time_span& pull_span, bool reconnect) {
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
		Assert(ring_->read_start_time() == pull_span.start_time());
		
		if(this_graph().was_stopped()) return pull_result::stopped;
		
		MF_DEBUG("output: pull ", pull_span, " : wait_readable...");
		ring_->wait_readable();
		MF_RAND_SLEEP;
		MF_DEBUG("output: pull ", pull_span, " : wait_readable. readable=", ring_->readable_duration());
	
		bool this_request_failed = (failed_request_id_ == current_request_id_);
		if(ring_->writer_reached_end()) {
			pull_span = span_intersection(pull_span, ring_->readable_time_span());
		}
		if(ring_->readable_duration() >= pull_span.duration()) {
			MF_DEBUG("output: pull -> success");
			return pull_result::success;
		} else if(this_graph().was_stopped()) {
			MF_DEBUG("output: pull -> stopped");
			return pull_result::stopped;
		} else if(this_request_failed) {
			MF_DEBUG("output: pull -> tfail");
			return pull_result::temporary_failure;
		}
	}

	MF_DEBUG("output: pull -> success");
	return pull_result::success;
}


timed_frame_array_view async_node::output_begin_read_(time_unit duration)  {
	return ring_->try_begin_read(duration);
}

void async_node::output_end_read_(time_unit duration) {
	ring_->end_read(duration);
}


}}
