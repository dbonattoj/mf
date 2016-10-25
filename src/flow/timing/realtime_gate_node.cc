#include "realtime_gate_node.h"
#include "../../utility/misc.h"

namespace mf { namespace flow {


void realtime_gate_node::thread_main_() {
	const stream_timing& input_timing = input().connected_node().output_timing();
	
	int i=1;
	for(;;) {
		clock_time_point now = clock_type::now();
		
		time_unit next_t = (now - launch_clock_time_).count() / input_timing.frame_clock_duration().count();
		set_current_time_(next_t);
		//set_current_time_(current_time()+1);
		
		input().pre_pull();
		pull_result pull_res = input().pull();
		
		{
			std::lock_guard<std::mutex> lock(mutex_);
			node_frame_window_view in_vw = input().begin_read_frame();
			back_buffer_->view()[0] = in_vw[0];
			input().end_read_frame();
		}
				
		std::cout << "loaded " << next_t << std::endl;

		//break;
	}
	
	std::cout << "ended " << std::endl;
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
	const auto& frame_frm = input().frame_format();
	front_buffer_.reset(new frame_buffer_type(make_ndsize(1), frame_frm));
	back_buffer_.reset(new frame_buffer_type(make_ndsize(1), frame_frm));
}


void realtime_gate_node::output_pre_pull_(const time_span&) {
	
}


node::pull_result realtime_gate_node::output_pull_(time_span& span) {
	static int i = 1;
	if(i--==1) sleep(3);

	{
		std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
		if(lock.owns_lock())
			front_buffer_->view() = back_buffer_->view();
	}

	//sleep(1);
	Assert(span.duration() == 1);
	current_output_time_ = span.start_time();
	return pull_result::success;
}


node_frame_window_view realtime_gate_node::output_begin_read_(time_unit duration) {
	Assert(duration == 1);
	
	return node_frame_window_view(timed_frame_array_view(front_buffer_->view(), current_output_time_));
}


void realtime_gate_node::output_end_read_(time_unit duration) {
}


}}
