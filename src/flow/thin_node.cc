#include "thin_node.h"

namespace mf { namespace flow {

void thin_node::internal_setup() {
	if(outputs().size() != 1) throw invalid_flow_graph("thin_node must have exactly 1 output");
	if(inputs().size() != 1) throw invalid_flow_graph("thin_node must have exactly 1 input");
	if(inputs().past_window_duration() != 0) throw invalid_flow_graph("thin_node input must have no past window");
	if(inputs().future_window_duration() != 0) throw invalid_flow_graph("thin_node input must have no futurer window");
	this->setup();	
}


void thin_node::launch() { }

void thin_node::stop() { }


void thin_node::pull(time_unit t) {
	// pull frame from input
	input_().pull(t);
	
	
	
	// read frame, get view
	auto vw = input_().begin_read(1);
	
	node_job job = make_job();
	job.define_time(t);
	job.push_output(output_(), vw[0]);
	job.push_input(input_(), vw);
	// TODO verify format compatibility

	this->process(job);
	
	job.pop_input();
	job.pop_output();
	bool reached_end = input_().reached_end(t);
	
	
	
	job.pop_input(reached_end);
}

void thin_node_output::pull(time_unit t) {
	this_node().pull(t);
}


timed_frames_view thin_node_output::begin_read(time_unit duration) {
	MF_ASSERT(duration == 1);
	return thin_node().
}


void thin_node_output::end_read(time_unit duration) {
	
}


time_unit thin_node_output::end_time() const {
	
}


frame_view thin_node_output::begin_write_frame(time_unit& t) {
	//
}


void thin_node_output::end_write_frame(bool was_last_frame) {
	//
}


void thin_node_output::cancel_write_frame() {
	//
}

}}
