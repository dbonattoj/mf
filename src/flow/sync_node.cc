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
#include "node_job.h"
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
	setup_filter();
}


void sync_node::launch() { }

void sync_node::stop() { }


bool sync_node::process_next_frame() {	
	//usleep(200000);

	node_job job = make_job();
	time_unit t;
	
	auto out_vw = ring_->begin_write(1);
	t = out_vw.start_time();

	if(end_time() != -1) MF_ASSERT(t < end_time());
	
	set_current_time(t);
	job.define_time(t);
		
	pre_process_filter(job);

	job.push_output(output(), out_vw[0]);
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		pull_result res = in->pull(t);
		if(res == pull_result::stopped) return false;
		else if(res == pull_result::temporary_failure) return false;
	}
	
	for(auto&& in : inputs()) if(in->is_activated()) {
		timed_frame_array_view in_view = in->begin_read_frame(t);
		MF_ASSERT(! in_view.is_null());
		MF_ASSERT(in_view.span().includes(t));
		
		job.push_input(*in, in_view);
	}

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
	
	job.pop_output();
	
	ring_->end_write(1);
	
	if(reached_end) mark_end();
	
	return true;
}


void sync_node::output_setup() {	
	node& connected_node = output().connected_node();
	
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);
		
	ndarray_generic_properties prop(output().format(), output().frame_length(), required_capacity);
	ring_.reset(new timed_ring(prop));
}


node::pull_result sync_node::output_pull(time_span& span, bool reconnected) {
	// if non-sequential: seek ring to new position
	time_unit ring_read_t = ring_->read_start_time();
	if(ring_read_t != span.start_time()) ring_->seek(span.start_time());
	
	if(reconnected) {
		set_current_time(span.start_time());
		set_online();
	}

	// pull frames from node until requested span filled, or end reached	
	while(!ring_->readable_time_span().includes(span) && ring_->write_start_time() != end_time()) {
		bool ok = process_next_frame();
		if(! ok) return pull_result::temporary_failure;
	}

	span = time_span(span.start_time(), span.start_time() + ring_->readable_time_span().duration());
	return pull_result::success;
}


timed_frame_array_view sync_node::output_begin_read(time_unit duration) {	
	if(end_time() != -1 && ring_->read_start_time() + duration > end_time())
		duration = ring_->readable_duration();

	MF_ASSERT(duration <= ring_->readable_duration());

	return ring_->begin_read(duration);
}


void sync_node::output_end_read(time_unit duration) {
	ring_->end_read(duration);
}

sync_node& sync_node_output::this_node() {
	return static_cast<sync_node&>(node_output::this_node());
}

const sync_node& sync_node_output::this_node() const {
	return static_cast<const sync_node&>(node_output::this_node());
}

void sync_node_output::setup() {
	this_node().output_setup();
}

node::pull_result sync_node_output::pull(time_span& span, bool reconnect) {
	return this_node().output_pull(span, reconnect);
}

timed_frame_array_view sync_node_output::begin_read(time_unit duration) {
	return this_node().output_begin_read(duration);
}

void sync_node_output::end_read(time_unit duration) {
	return this_node().output_end_read(duration);
}

time_unit sync_node_output::end_time() const {
	return this_node().end_time();
}


sync_node_output& sync_node::output() { return static_cast<sync_node_output&>(*outputs().front()); }
const sync_node_output& sync_node::output() const { return static_cast<const sync_node_output&>(*outputs().front()); }



node_output& sync_node::add_output(const frame_format& format) {
	return add_output_<sync_node_output>(format);
}

}}
