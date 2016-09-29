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

#include "sync_node.h"
#include "../node_graph.h"
#include <unistd.h>

namespace mf { namespace flow {

sync_node::sync_node(node_graph& gr) :
	processing_node(gr, true) { }


time_unit sync_node::minimal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;
	const node_input& in = output().connected_input();
	return in.this_node().minimal_offset_to(target_node) - in.past_window_duration();
}


time_unit sync_node::maximal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;	
	const node_input& in = output().connected_input();
	return in.this_node().maximal_offset_to(target_node) + in.future_window_duration();
}


thread_index sync_node::processing_thread_index() const {
	return output().reader_thread_index();
}


void sync_node::setup() {		
	node& connected_node = output().connected_node();
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);
		
	auto buffer_frame_format = output_frame_format_();
	ring_.reset(new timed_ring(buffer_frame_format, required_capacity));
}


bool sync_node::process_next_frame_() {
	// Begin writing 1 frame to output buffer
	auto output_write_handle = ring_->write(1); // if error occurs, handle will cancel write during stack unwinding
	const timed_frame_array_view& out_vw = output_write_handle.view();
	
	// Start time of the output view determined new current time of this node
	time_unit t = out_vw.start_time();	
	set_current_time_(t);
	
	// Create job
	processing_node_job job(*this, std::move(current_parameter_valuation_())); // reads current time of node
	auto cancel_output = []() { };
	job.attach_output_view(out_vw[0], cancel_output);
	
	// Let handler pre-process frame
	handler_result handler_res = handler_pre_process_(job);
	if(handler_res == handler_result::failure) return process_result::failure;
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
		else if(res == pull_result::fatal_failure) return process_result::failure;
		else if(res == pull_result::stopped) return process_result::failure;
		else if(res == pull_result::end_of_stream) return process_result::end_of_stream;
	}
	
	// Begin reading from the activated inputs (in job object)
	// If error occurs, job object will cancel read during stack unwinding
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(in.is_activated()) job.begin_input(in);
	}

	// Let handler process frame
	handler_res = handler_process_(job);
	if(handler_res == handler_result::failure) return process_result::failure;
	else if(handler_res == handler_result::end_of_stream) return process_result::end_of_stream;

	// End reading from the inputs 
	job.end_inputs();
	
	// Detach output from output write handle,
	// and commit 1 written frame to output buffer
	job.detach_output();
	output_write_handle.end(1);
	
	// Finish the job
	update_parameters_(job.parameters()); // TODO move instead of copy
		
	// Processing the frame succeeded
	return process_result::success;
}


void sync_node::output_pre_pull_(const time_span&) {	
	// Does nothing in sync_node
}


node::pull_result sync_node::output_pull_(time_span& span) {
	// On non-sequential pull, seek output buffer to new time
	if(ring_->read_start_time() != span.start_time()) ring_->seek(span.start_time());
	Assert(ring_->read_start_time() == span.start_time());
		
	// Sequentially process frames until span is in buffer
	process_result process_res = process_result::success;
	while(! ring_->readable_time_span().includes(span)) {
		process_res = process_next_frame_();
		if(process_res != process_result::success) {
			// Failure when processing: span to pull ends earlier than requested
			// (current_time() frame was not processed)
			span.set_end_time(current_time());
			break;
		}
	}
	
	// Return pull result
	if(process_res == process_result::success) {
		// Success: pulled span must be readable in buffer, ready to be read using output_begin_read_
		Assert(ring_->readable_duration() >= span.duration());
		return pull_result::success;
	} else {
		// Failure: return error state, and span may have been truncated		
		if(graph().was_stopped()) return pull_result::stopped;
		else if(process_res == process_result::transitory_failure) return pull_result::transitory_failure;
		else if(process_res == process_result::handler_failure) return pull_result::handler_failure;
		else if(process_res == process_result::end_of_stream) return pull_result::end_of_stream;
	}
}

	
node_frame_window_view sync_node::output_begin_read_(time_unit duration) {
	Assert(ring_->readable_duration() >= duration);
	return ring_->begin_read(duration);
}


void sync_node::output_end_read_(time_unit duration) {
	ring_->end_read(duration);
}

}}
