#include "sync_node.h"

namespace mf { namespace flow {

void sync_node::internal_setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("sync_node must have exactly 1 output");
	this->setup();
}


void sync_node::launch() { }

void sync_node::stop() { }


void sync_node::process_next_frame() {	
	node_job job = make_job();
	time_unit t;
	
	node_output& out = outputs().front();
	auto out_view = out.begin_write_frame(t);
	MF_ASSERT(! out_view.is_null());
	if(end_time() != -1) MF_ASSERT(t < end_time());
		
	set_current_time(t);
	
	this->pre_process(t);

	job.define_time(t);
	job.push_output(out, out_view);
	
	bool stopped = false;
	for(node_input& in : inputs()) {
		MF_ASSERT(! in.reached_end(t));
		if(in.is_activated()) {
			in.pull(t);
			
			timed_frames_view in_view = in.begin_read_frame(t);
			MF_ASSERT(in_view.span().includes(t));
			if(in_view.is_null()) { stopped = true; break; }
			
			job.push_input(in, in_view);
		}
	}
	
	if(stopped) {
		cancel_job(job);
		return;
	}

	this->process(job);
	
	bool reached_end = false;
	if(stream_duration_is_defined()) {
		if(t == stream_duration() - 1) reached_end = true;
		MF_ASSERT(t < stream_duration());
	} else if(is_source()) {
		reached_end = job.end_was_marked();
	}
	
	while(node_input* in = job.pop_input()) {
		in->end_read_frame(t);
		if(in->reached_end(t)) reached_end = true;
	}
	
	job.pop_output()->end_write_frame(reached_end);

	MF_DEBUG_EXPR_T("node", name, reached_end, t);

	if(reached_end) mark_end();
}


void sync_node_output::setup() {
	node& connected_node = connected_input().this_node();
	
	time_unit offset_diff = this_node().offset() - connected_node.offset();
	time_unit required_capacity = 1 + connected_input().past_window_duration() + offset_diff;
	
	frame_array_properties prop(format(), frame_length(), required_capacity);
	ring_.reset(new timed_ring(prop));
}


void sync_node_output::pull(time_span span) {
	// if non-sequential: seek ring to new position
	time_unit ring_read_t = ring_->read_start_time();
	if(ring_read_t != span.start_time()) ring_->seek(span.start_time());

	// pull frames from node until requested span filled, or end reached	
	while(!ring_->readable_time_span().includes(span) && ring_->write_start_time() != this_node().end_time()) {
		this_node().process_next_frame();
	}
}


timed_frames_view sync_node_output::begin_read(time_unit duration) {
	time_unit end_time = this_node().end_time();
	
	if(end_time != -1 && ring_->read_start_time() + duration > end_time)
		duration = ring_->readable_duration();

	MF_ASSERT(duration <= ring_->readable_duration());

	return ring_->begin_read(duration);
}


void sync_node_output::end_read(time_unit duration) {
	ring_->end_read(duration);
}


time_unit sync_node_output::end_time() const {
	return this_node().end_time();
}


frame_view sync_node_output::begin_write_frame(time_unit& t) {
	auto view = ring_->begin_write(1);
	t = view.start_time();
	return view[0];
}


void sync_node_output::end_write_frame(bool was_last_frame) {
	ring_->end_write(1);
}


void sync_node_output::cancel_write_frame() {
	ring_->end_write(0);
}

}}
