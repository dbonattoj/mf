#include "sync_node.h"

namespace mf { namespace flow {

void sync_node_output_channel::setup(time_unit required_capacity) {
	ndarray_generic_properties prop(format(), frame_length(), required_capacity);
	
}


frame_view sync_node_output_channel::begin_write_frame(time_unit expected_time) {
	timed_frame_array_view vw = ring_->begin_write(1);
	if(vw.start_time() == expected_time) {
		return vw[0];
	} else {
		ring_->end_write(0);
		return frame_view::null();
	}
}


void sync_node_output_channel::end_write_frame() {
	ring_->end_write(1);
}


void sync_node_output_channel::cancel_write_frame() {
	ring_->end_write(0);
}


timed_frame_array_view sync_node_output_channel::begin_read(time_unit duration) {
	Expects(ring_->readable_duration() >= duration);
	
	if(reached_end() && ring_->read_start_time() + duration > current_time())
		duration = ring_->readable_duration();

	Assert(duration <= ring_->readable_duration());

	return ring_->begin_read(duration);
}


void sync_node_output_channel::end_read(time_unit duration) {
	ring_->end_read(duration);
}


///////////////


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
	const node& connected_node = output().connected_node();
	time_unit required_capacity = 1 + maximal_offset_to(connected_node) - minimal_offset_to(connected_node);
	
	lowest_capacity_channel_index_ = -1;
	time_unit lowest_capacity_;

	for(std::ptrdiff_t i = 0; i < output_channels_count(); ++i) {
		output_channel_at(i).setup(required_capacity);
		time_unit capacity = output_channel_at(i).this_ring().total_duration();
	
		if(lowest_capacity_channel_index_ == -1 || capacity < lowest_capacity_) {
			lowest_capacity_ = cap;
			lowest_capacity_channel_index_ = i;
		}
	}
	
}


time_span sync_node::readable_time_span_() const {
	return output_channel_at(lowest_capacity_channel_index_).this_ring().readable_time_span();
}


time_unit sync_node::write_start_time_() const {
	return output_channel_at(lowest_capacity_channel_index_).this_ring().write_start_time();
}


void sync_node::seek_(time_unit t) {
	for(std::ptrdiff_t i = 0; i < output_channels_count(); ++i)
		output_channel_at(i).this_ring().seek(t);
}


node::pull_result sync_node::output_pull_(time_span& span, bool reconnected) {
	if(readable_time_span_().start_time() != span.start_time()) seek_(span.start_time());
	Assert(readable_time_span_().start_time() == span.start_time());
	
	if(reached_end() && span.end_time() > end_time())
		span = time_span(span.start_time(), end_time());

	if(span.duration() == 0) return pull_result::success;
	
	bool cont = true;
	if(readable_time_span_().includes(span)) {
		if(reconnected) set_online();
	} else {
		if(reconnected) {
			set_current_time_(write_start_time_());
			set_online();
		}
		do {
			cont = process_next_frame_(write_start_time_());
		} while(cont & !readable_time_span_().includes(span) && !reached_end());
	}
	
	if(reached_end() && span.end_time() > end_time())
		span = time_span(span.start_time(), end_time());
	
	if(cont) {
		Ensures(ring_->readable_duration() >= span.duration());
		return pull_result::success;
	} else if(this_graph().was_stopped()) {
		return pull_result::stopped;
	} else {
		return pull_result::transitory_failure;
	}
}


bool sync_node::process_next_frame_(time_unit t) {
	if(stream_properties().duration_is_defined()) Assert(t < stream_properties().duration());

	set_current_time_(t);
	processing_node_job job = begin_job_();
	
	for(std::ptrdiff_t i = 0; i < output_channels_count(); ++i) {
		bool ok = job.begin_output(output_channel_at(i));
		Assert(ok);
	}
	
	handler_pre_process_(job);
	
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		pull_result res = input_at(i).pull();
		if(res == pull_result::stopped || res == pull_result::transitory_failure) return false;
	}
	
	handler_process_(job);
	
	finish_job_(job);
	
	return true;
}


}}
