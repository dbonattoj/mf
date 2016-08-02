#include "node_input.h"
#include "node_input.h"

namespace mf { namespace flow {

node_input::node_input(node& nd) :
	node_(nd) { }


void node_input::connect(node_remote_output& output, std::ptrdiff_t channel_index) {
	Expects(connected_output_ == nullptr, "cannot connect node_input when already connected");
	Expects(! output.this_output().is_connected(), "cannot connect the input to an output that is already connected");
	connected_output_ = &output;
	connected_output_->this_output().input_has_connected(*this);
}


void node_input::disconnect() {
	Expects(connected_output_ != nullptr, "cannot disconnect node_input when not connected");
	connected_output_->this_output().input_has_disconnected();
	connected_output_ = nullptr;
}


node::pull_result node_input::pull() {
	Expects(is_connected());
	
	time_unit t = this_node().current_time();
	time_unit start_time = std::max(time_unit(0), t - past_window_);
	time_unit end_time = t + future_window_ + 1;
	
	bool reconnect = (this_node().state() == node::online) && (connected_node().state() == node::reconnecting);
	// TODO check when connected_node().state() can change
	
	time_span span = time_span(start_time, end_time);
	node::pull_result result = connected_output_->pull(span, reconnect);
	Assert(span.start_time() == start_time);
	Assert(span.includes(t));
	
	pulled_span_ = span;
	return result;
}


timed_frame_array_view node_input::begin_read_frame(std::ptrdiff_t channel_index) {
	Expects(pulled_span_.duration() > 0);

	time_unit t = this_node().current_time();
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	if(duration > pulled_span_.duration()) duration = pulled_span_.duration();
	
	//duration = pulled_span_.duration();
	
	timed_frame_array_view view = connected_output_->begin_read(duration, channel_index);
	Assert(! view.is_null());
	
	if(view.is_null()) return view;
	
	Assert(view.span().includes(t));
	Assert(view.start_time() == pulled_span_.start_time());
	Assert(view.duration() == duration);
	return view;
}


void node_input::end_read_frame(std::ptrdiff_t channel_index) {
	time_unit duration = (this_node().current_time() < past_window_) ? 0 : 1;
	connected_output_->end_read(duration, channel_index);
}


void node_input::cancel_read_frame(std::ptrdiff_t channel_index) {
	connected_output_->end_read(0, channel_index);
}


void node_input::set_activated(bool activated) {
	if(activated_ != activated) {
		activated_ = activated;
		if(activated) connected_node().propagate_reconnecting_state();
		else connected_node().propagate_offline_state();
	}
}

}}
