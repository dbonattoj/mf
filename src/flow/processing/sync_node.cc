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
	timed_frame_array_view out_vw = ring_->begin_write(1);
	time_unit t = out_vw.start_time();
	
	if(stream_timing().has_duration()) Assert(t < stream_timing().duration());
	
	set_current_time_(t);
	processing_node_job job = begin_job_();
	
	job.attach_output_view(out_vw[0]);
	
	handler_pre_process_(job);
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		in.pre_pull();
	}
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		
		pull_result res = in.pull();
		if(res == pull_result::stopped || res == pull_result::transitory_failure) {
			job.detach_output_view();
			return false;
		}
	}
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
	
		bool cont = job.begin_input(in);
	}


	handler_process_(job);
			
	job.detach_output_view();
	ring_->end_write(1);

	finish_job_(job);
		
	return true;
}


void sync_node::output_pre_pull_(const time_span& span) {	

}


node::pull_result sync_node::output_pull_(time_span& span) {	
	if(ring_->read_start_time() != span.start_time()) ring_->seek(span.start_time());
	Assert(ring_->read_start_time() == span.start_time());
	
	if(reached_end() && span.end_time() > end_time())
		span = time_span(span.start_time(), end_time());

	if(span.duration() == 0) return pull_result::success;
	
	bool cont = true;
	
/*	while(cont && !ring_->readable_time_span().includes(span) && !reached_end())
		cont = process_next_frame_();
	*/	
	
	if(ring_->readable_time_span().includes(span)) {

	} else {
		do {
			cont = process_next_frame_();
		} while(cont && !ring_->readable_time_span().includes(span) && !reached_end());
	}
	

	if(reached_end() && span.end_time() > end_time())
		span = time_span(span.start_time(), end_time());

	if(cont) {
		Assert(ring_->readable_duration() >= span.duration());
		return pull_result::success;
	} else {
		if(graph().was_stopped()) return pull_result::stopped;
		else return pull_result::transitory_failure;
	}
}

	
node_frame_window_view sync_node::output_begin_read_(time_unit duration) {
	Expects(ring_->readable_duration() >= duration);
	
	if(reached_end() && ring_->read_start_time() + duration > current_time())
		duration = ring_->readable_duration();

	MF_ASSERT(duration <= ring_->readable_duration());

	return ring_->begin_read(duration);
}


void sync_node::output_end_read_(time_unit duration) {
	ring_->end_read(duration);
}

}}
