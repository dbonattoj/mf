#include "node.h"

namespace mf { namespace flow {



node::job::job(node& nd, time_unit t) : 
	node_(nd),
	time_(t),
	input_views_(nd.inputs_.size()),
	output_views_(nd.outputs_.size()) { }


node::job::~job() {
	close_all();
}


void node::job::open(const node_input& port) {
	timed_frames_view view = port.begin_read_frame(time_);
	input_views_[port.index()].reset(view);
}


void node::job::open(const node_output& port) {
	std::ptrdiff_t index = port.index();
	time_unit t = -1;
	frame_view view = port.begin_write_frame(t);
	if(t != time_) throw sequencing_error("time mismatch between node job time, and node output write frame time");
	output_views_[port.index()].reset(view);
}


void node::job::close_all() {
	for(const timed_frames_view& input_view : input_views_) if(input_view) {
		node_.inputs_[input_view.index()].end_read_frame(time_);
		input_view.reset();
	}
	for(const frame_view& output_view : output_view_) if(output_view) {
		node_.outputs_[output_view.index()].end_write_frame(end_marked_);
		output_view.reset();
	}
}


void node::job::mark_end() {
	end_marked_ = true;
}


/////


void node_input::connect(node_output& output) {
	connected_output_ = &output;
}

void node_input::pull(time_unit t) {
	MF_EXPECTS(is_connected());

	time_unit start_time = std::max(0, t - past_window_);
	connected_output().pull(start_time);
	MF_ASSERT(connected_node().pull_time() == start_time);
}

timed_frames_view node_input::begin_read_frame(time_unit t) {
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	timed_frames_view view = connected_output().begin_read(duration);
	if(view) MF_ASSERT(view.start_time() == connected_node().pull_time());
	return view;
}

void node_input::end_read_frame(time_unit t) {
	time_unit duration = (t < past_window_) ? 0 : 1;
	connected_output().end_read(duration);
}

}}

