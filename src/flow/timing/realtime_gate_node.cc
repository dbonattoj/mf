#include "realtime_gate_node.h"

namespace mf { namespace flow {


std::size_t realtime_gate_node_output::channels_count() const noexcept override {
	return this_node().input().channels_count();
}


std::string realtime_gate_node_output::channel_name_at(std::ptrdiff_t i) const override {
	return this_node().input().channel_name_at(i);
}


node::pull_result realtime_gate_node_output::pull(time_span& span, bool reconnect) override {
	return this_node().output_pull(span, reconnect);
}


node_frame_window_view realtime_gate_node_output::begin_read(time_unit duration) override {
	return this_node().output_begin_read(duration);
}


void realtime_gate_node_output::end_read(time_unit duration) override {
	return this_node().output_end_read(duration);
}


///////////////


stream_timing realtime_gate_node::deduce_output_stream_timing_() const {
	stream_timing timing;
	timing.set_real_time(true);
	return timing;
}


void realtime_gate_node::thread_main_() {
	const node_stream_timing& timing = stream_timing();
	
	for(;;) {
		clock_time_point now = clock_type::now();
		
		time_unit next_t = (now - launch_clock_time_).count() / timing.frame_clock_duration().count();
		set_current_time_(next_t);
		
		input().pre_pull();
		pull_result pull_res = input().pull();
		
		node_frame_window_view in_vw = input().begin_read_frame();
		loaded_frame_ = frame_buffer_type(in_vw[0]);
		input().end_read_frame();
		
		{
			std::lock_guard<std::mutex> lock(mutex_);
			std::swap(outputted_frame_, loaded_frame_);
		}
	}
}


void realtime_gate_node::load_new_frame_() {
	
}


realtime_gate_node::realtime_gate_node(node_graph& gr) :
	base(gr)
{
	add_input_(*this);
	add_output_(*this);
}
	

void realtime_gate_node::launch() override {
	launch_clock_time_ = clock_type::now();
	thread_ = std::move(std::thread(
		std::bind(&realtime_gate_node::thread_main_, this)
	));
}


void realtime_gate_node::pre_stop() override {
	
}


void realtime_gate_node::stop() override {
	
}


void realtime_gate_node::pre_setup() override {
	
}


void realtime_gate_node::setup() override {
}


void realtime_gate_node::output_pre_pull_(const time_span&) {
	
}


node::pull_result realtime_gate_node::output_pull_(time_span& span) {
	Assert(span.duration() == 1);
}


node_frame_window_view realtime_gate_node::output_begin_read_(time_unit duration) {
	Assert(duration == 1);
	mutex_.lock();
	return outputted_frame_.view();
}


void realtime_gate_node::output_end_read_(time_unit duration) {
	mutex_.unlock();
}


}}
