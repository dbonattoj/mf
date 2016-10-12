#include "realtime_gate_node.h"

namespace mf { namespace flow {


void realtime_gate_node::thread_main_() {
	const stream_timing& input_timing = input().connected_node().output_stream_timing();
	
	for(;;) {
		clock_time_point now = clock_type::now();
		
		time_unit next_t = (now - launch_clock_time_).count() / input_timing.frame_clock_duration().count();
		set_current_time_(next_t);
		
		input().pre_pull();
		pull_result pull_res = input().pull();
		
		node_frame_window_view in_vw = input().begin_read_frame();
		buffers_->loaded_frame[0] = in_vw[0];
		input().end_read_frame();
		
		{
			std::lock_guard<std::mutex> lock(mutex_);
			std::swap(buffers_->outputted_frame, buffers_->loaded_frame);
		}
	}
}


void realtime_gate_node::load_new_frame_() {
	
}


realtime_gate_node::realtime_gate_node(node_graph& gr) :
	gate_node(gr, stream_timing::real_time()) { }
	

void realtime_gate_node::launch() {
	launch_clock_time_ = clock_type::now();
	thread_ = std::move(std::thread(
		std::bind(&realtime_gate_node::thread_main_, this)
	));
}


void realtime_gate_node::pre_stop() {
	
}


void realtime_gate_node::stop() {
	
}


void realtime_gate_node::pre_setup() {
	
}


void realtime_gate_node::setup() {
	buffers_.reset(new buffers( input().frame_format() ));
}


void realtime_gate_node::output_pre_pull_(const time_span&) {
	
}


node::pull_result realtime_gate_node::output_pull_(time_span& span) {
	Assert(span.duration() == 1);
	return pull_result::success;
}


node_frame_window_view realtime_gate_node::output_begin_read_(time_unit duration) {
	Assert(duration == 1);
	mutex_.lock();
	return node_frame_window_view(timed_frame_array_view(buffers_->outputted_frame.view(), 0));
}


void realtime_gate_node::output_end_read_(time_unit duration) {
	mutex_.unlock();
}


}}
