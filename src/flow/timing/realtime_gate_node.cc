#include "realtime_gate_node.h"
#if 0
namespace mf { namespace flow {


std::size_t realtime_gate_node_output::channels_count() const noexcept override {
	
}


std::string realtime_gate_node_output::channel_name_at(std::ptrdiff_t i) const override {
	
}


node::pull_result realtime_gate_node_output::pull(time_span& span, bool reconnect) override {
	
}


node_frame_window_view realtime_gate_node_output::begin_read(time_unit duration) override {
	
}


void realtime_gate_node_output::end_read(time_unit duration) override {
	
}


///////////////



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


/*
sync_node::process_result sync_node::process_next_frame_() {
	// Begin writing 1 frame to output buffer
	auto output_write_handle = ring_->write(1); // if error occurs, handle will cancel write during stack unwinding
	const timed_frame_array_view& out_vw = output_write_handle.view();
	
	// Start time of the output view determined new current time of this node
	time_unit t = out_vw.start_time();	
	set_current_time_(t);
	
	// Create job
	processing_node_job job(*this, std::move(current_parameter_valuation_())); // reads current time of node
	job.attach_output(out_vw[0], nullptr);
	
	// Let handler pre-process frame
	handler_result handler_res = handler_pre_process_(job);
	if(handler_res == handler_result::failure) return process_result::handler_failure;
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
		else if(res == pull_result::fatal_failure) return process_result::handler_failure;
		else if(res == pull_result::stopped) return process_result::stopped;
		else if(res == pull_result::end_of_stream) return process_result::end_of_stream;
	}
	
	// Begin reading from the activated inputs (in job object)
	// If error occurs, job object will cancel read during stack unwinding
	for(std::ptrdiff_t i = 0; i < inputs_count(); ++i) {
		input_type& in = input_at(i);
		if(! in.is_activated()) continue;
		bool began = job.begin_input(in);
		if(! began) return process_result::transitory_failure;
	}

	// Let handler process frame
	handler_res = handler_process_(job);
	if(handler_res == handler_result::failure) return process_result::handler_failure;
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
*/

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
	//std::lock_guard<std::mutex> lock(mutex_);
	return outputted_frame_.view();
}


void realtime_gate_node::output_end_read_(time_unit duration) {
	
}


}}
#endif
