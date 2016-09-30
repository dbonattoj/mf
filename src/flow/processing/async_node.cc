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
#include "../node_graph.h"

#include <iostream>

namespace mf { namespace flow {
	
async_node::async_node(node_graph& gr) :
	processing_node(gr, true)
{
	thread_index_ = graph().new_thread_index();
}
	

async_node::~async_node() {
	Assert(! running_);
	Assert(! thread_.joinable());
}


thread_index async_node::processing_thread_index() const {
	return thread_index_;
}


void async_node::setup() {		
	node& connected_node = output().connected_node();
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);
	
	auto buffer_frame_format = output_frame_format_();
	ring_.reset(new shared_ring(buffer_frame_format, required_capacity));
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
	return in.this_node().maximal_offset_to(target_node) + in.future_window_duration() + prefetch_duration_;
}


void async_node::pause_() {
	// pause as long as either:
	// - next write time (determined by shared_ring, and reader may seek) is beyond time limit (set by reader)
	// - current request had failed, not yet received new request from reader
	// conditions are checked when continuation_cv_ gets notified

	std::unique_lock<std::mutex> lock(continuation_mutex_);
	continuation_cv_.wait(lock, [&] {
		time_unit next_write_time = ring_->write_start_time();
		if(graph().was_stopped()) return true; // thread_main() will exit
		else if(failed_request_id_ != -1 && current_request_id_ == failed_request_id_) return false;
		else if(next_write_time >= time_limit_) return false;
		return true;
	});
}


void async_node::thread_main_() {
	bool pause = false;
	
	for(;;) {
		if(pause) {
			pause_();
			if(graph().was_stopped()) break;
			pause = false;
		}
		MF_DEBUG_T("async", name(), " continuation");

		request_id_type processing_request_id = current_request_id_.load();
		
		process_result process_res = process_frame_();
		if(process_res == process_result::should_pause) {
			pause = true;
		} else if(process_res != process_result::success) {
			failed_request_id_ = processing_request_id;
			failed_request_process_result_ = process_res;
			ring_->break_reader();
			pause = true;
		}
		
		MF_DEBUG_T("async", name(), " proccess_result not success/should_pause");
	}
	
	MF_DEBUG_T("async", name(), " END");
}


async_node::process_result async_node::process_frame_() {	
	// Begin writing 1 frame to output buffer
	// If currently no space in buffer, go to pause state (don't block here instead)
	auto output_write_handle = ring_->try_write(1);	
	if(output_write_handle.view().is_null()) return process_result::should_pause;
	const timed_frame_array_view& out_vw = output_write_handle.view();
		
	// t = time of frame to process here
	time_unit t = output_write_handle.view().start_time();

	// as long as output_write_handle is open, ring cannot seek to another time (reader will wait instead)
	// but in output_pre_pull_, reader sets time limit before seeking
	// writer may notice here that reader wants to seek
	if(t > time_limit_) return process_result::should_pause;
	
	// Set current time, create job
	set_current_time_(t);
	processing_node_job job(*this, std::move(current_parameter_valuation_())); // reads current time of node
	job.attach_output(out_vw[0], nullptr);

	// Let handler pre-process frame
	handler_result handler_res = handler_pre_process_(job);
	if(handler_res == handler_result::failure) return process_result::handler_failure;
	else if(handler_res == handler_result::end_of_stream) return process_result::end_of_stream;

	// Pre-pull the activated inputs
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(in.is_activated()) in.pre_pull();
	}

	// Pull the activated inputs
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		
		pull_result res = in.pull();
				
		if(res == pull_result::transitory_failure) return process_result::transitory_failure;
		else if(res == pull_result::fatal_failure) return process_result::handler_failure;
		else if(res == pull_result::stopped) return process_result::stopped;
		else if(res == pull_result::end_of_stream) return process_result::end_of_stream;
	}

	// Begin reading from the activated inputs (in job object)
	// If error occurs, job object will cancel read during stack unwinding
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(in.is_activated()) job.begin_input(in);
	}
	
	MF_DEBUG_T("async", name(), " process... handler_proc");


	// Let handler process frame
	handler_res = handler_process_(job);
	if(handler_res == handler_result::failure) return process_result::handler_failure;
	else if(handler_res == handler_result::end_of_stream) return process_result::end_of_stream;

	// Detach output from output write handle,
	// and commit 1 written frame to output buffer
	job.detach_output();
	output_write_handle.end(1);

	// Finish the job
	update_parameters_(job.parameters()); // TODO move instead of copy
		
	// Processing the frame succeeded
	return process_result::success;
}


void async_node::output_pre_pull_(const time_span& pull_span) {
	{
		// if writer currently paused, it won't continue until this lock is released again
		std::lock_guard<std::mutex> lock(continuation_mutex_);
	
		// set new time limit for writer
		time_limit_.store(pull_span.end_time() + prefetch_duration_ + 1);
				
		// seek shared_ring to new start time
		// if writer is currently processing a frame, this waits until it has written that frame (or failed on it)
		ring_->seek(pull_span.start_time());
		
		// set unique request id for the upcoming pull
		// necessary to determine if a failure in the writer concerned the request that reader was waiting for in
		// output_pull_().
		// failure could also occur during prefetch following the previous request, and then without a unique id reader
		// might be notified about the failure only after having started a new request (if the flag gets set too late)
		// then transitional failure could be propagated incorrectly, instead of just pausing the prefetch in this node
		current_request_id_++; // will overflow at maximal value
	}
	
	
	MF_RAND_SLEEP;
	continuation_cv_.notify_one();
}


node::pull_result async_node::output_pull_(time_span& pull_span) {
	while(ring_->readable_duration() < pull_span.duration()) {
																			MF_RAND_SLEEP;
		Assert(ring_->read_start_time() == pull_span.start_time());

		if(graph().was_stopped()) return pull_result::stopped;
		
		// let the writer process frames
		// wait until pull_span.duration() frames have become readable in shared_ring,
		// or writer failure occured, and shared_ring reader break event got sent
		bool frame_became_readable = ring_->wait_readable(); 				MF_RAND_SLEEP;
		if(frame_became_readable) continue;
																			MF_RAND_SLEEP;	
		// wait_readable got interrupted by reader break event
		// may be from process failure for this request,
		// or from the previous request (break coming in late)
		
		MF_DEBUG_T("async", name(), " pull -> fail", failed_request_id_, " <?> curr", current_request_id_.load());
		
		if(failed_request_id_ != current_request_id_) continue;
		// in latter case, continue waiting - writer will continue with this request
																			MF_RAND_SLEEP;
		// this request has failed...
		process_result process_res = failed_request_process_result_;
		pull_span.set_end_time(current_time());
		
		MF_DEBUG_T("async", name(), " pull -> fail ", pull_span);
		
		if(process_res == process_result::transitory_failure) return pull_result::transitory_failure;
		else if(process_res == process_result::handler_failure) return pull_result::fatal_failure;
		else if(process_res == process_result::end_of_stream) return pull_result::end_of_stream;
		else if(process_res == process_result::stopped) return pull_result::stopped;
		else throw std::logic_error("invalid process_result state in failed_request_process_result_");
	}
	
	MF_DEBUG_T("async", name(), " pull -> ok ", pull_span);
	return pull_result::success;
}


node_frame_window_view async_node::output_begin_read_(time_unit duration) {
	Assert(ring_->readable_duration() >= duration);
	return ring_->begin_read(duration);
}

void async_node::output_end_read_(time_unit duration) {
	ring_->end_read(duration);
}


}}
