#include "sync_node.h"

namespace mf { namespace flow {

void sync_node::internal_setup() {
	this->setup();
}


void sync_node::launch() { }

void sync_node::stop() { }


void sync_node::pull(time_unit t) {
	node_job job = make_job();
	
	time_unit out_t;
	node_output& out = outputs().front();
	auto out_view = out.begin_write_frame(t);
	///////////
}


void sync_node_output::setup() {
	node_input& connected_input = connected_input();
	node& connected_node = connected_input.this_node();
	
	time_unit offset_diff = connected_node.offset() - this_node().offset();
	time_unit required_capacity = 1 + connected_input.past_window_duration() + offset_diff;
	
	frame_array_properties prop(format(), frame_length(), required_capacity);
	ring_.reset(new timed_ring(prop));
}


void sync_node_output::pull(time_span span) {
	time_unit ring_read_t = ring_->read_start_time();
	if(ring_read_t != span.start_time()) ring_->seek(span.start_time());

	for(time_unit t = span.start_time(); t < span.end_time() ++t)
		this_node().pull(t);
}


timed_frames_view sync_node_output::begin_read(time_unit duration) {
	time_unit ring_read_t = ring_->read_start_time();
	if(t != ring_read_t) {
		
	}
}


void sync_node_output::end_read(t duration) {
	
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
