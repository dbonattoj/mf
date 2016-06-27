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
#include "graph.h"
#include <unistd.h>

namespace mf { namespace flow {

time_unit sync_node::minimal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;
	const node_input& in = output().connected_input();
	return in.this_node().minimal_offset_to(target_node) - in.past_window_duration();
}

time_unit sync_node::maximal_offset_to(const node& target_node) const {
	if(&target_node == this) return 0;
	const node_input& in = output().connected_input();
	return in.this_node().minimal_offset_to(target_node) + in.future_window_duration();
}

void sync_node::setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("sync_node must have exactly 1 output");
	setup_filter_();
	
	node& connected_node = output().connected_node();
	
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);
		
	ndarray_generic_properties prop(output().format(), output().frame_length(), required_capacity);
	ring_.reset(new timed_ring(prop));
}


node::pull_result sync_node::output_pull_(time_span& span, bool reconnected) {
	// if non-sequential: seek ring to new position
	time_unit ring_read_t = ring_->read_start_time();
	if(ring_read_t != span.start_time()) ring_->seek(span.start_time());
	
	if(reconnected) {
		set_current_time_(span.start_time());
		set_online();
	}

	// pull frames from node until requested span filled, or end reached	
	while(!ring_->readable_time_span().includes(span) && ! reached_end()) {
		bool ok = process_next_frame_();
		if(! ok) return pull_result::temporary_failure;
	}

	span = time_span(span.start_time(), span.start_time() + ring_->readable_time_span().duration());
	return pull_result::success;
}

	
timed_frame_array_view sync_node::output_begin_read_(time_unit duration) {
	if(reached_end() && ring_->read_start_time() + duration > current_time())
		duration = ring_->readable_duration();

	MF_ASSERT(duration <= ring_->readable_duration());

	return ring_->begin_read(duration);
}


void sync_node::output_end_read_(time_unit duration) {
	ring_->end_read(duration);
}


bool sync_node::process_next_frame_() {
	timed_frame_array_view out_vw = ring_->begin_write(1);
	time_unit t = out_vw.start_time();
	filter_node_job job = make_job_(t);
	job.attach_output(out_vw[0]);
	
	set_current_time_(t);
	
	pre_process_filter_(job);
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		pull_result res = in->pull(t);
		if(res == pull_result::stopped || res == pull_result::temporary_failure) {
			job.detach_output();
			return false;
		}
	}
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		job.push_input(*in);
	}
	
	process_filter_(job);
	
	if(stream_properties().duration_is_defined()) {
		if(t == stream_properties().duration() - 1) mark_end_();
	} else if(job.end_was_marked()) {
		mark_end_();
	}
	
	while(job.has_inputs()) {
		const node_input& in = job.pop_input();
		if(t == in.end_time() - 1) mark_end_();
	}
	
	job.detach_output();
	ring_->end_write(1);
	
	return true;
}


}}
