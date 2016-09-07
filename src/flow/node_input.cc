/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "node_input.h"
#include "node_input.h"

namespace mf { namespace flow {

node_input::node_input(node& nd) :
	node_(nd) { }


void node_input::connect(node_output& output) {
	Expects(connected_output_ == nullptr, "cannot connect node_input when already connected");
	Expects(! output.is_connected(), "cannot connect the input to an output that is already connected");
	connected_output_ = &output;
	connected_output_->input_has_connected(*this);
}


void node_input::disconnect() {
	Expects(connected_output_ != nullptr, "cannot disconnect node_input when not connected");
	connected_output_->input_has_disconnected();
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


timed_frame_array_view node_input::begin_read_frame() {
	Expects(pulled_span_.duration() > 0);

	time_unit t = this_node().current_time();
	time_unit duration = std::min(t, past_window_) + 1 + future_window_;
	if(duration > pulled_span_.duration()) duration = pulled_span_.duration();
	
	//duration = pulled_span_.duration();
	
	timed_frame_array_view view = connected_output_->begin_read(duration);
	Assert(! view.is_null());
	
	if(view.is_null()) return view;
	
	Assert(view.span().includes(t));
	Assert(view.start_time() == pulled_span_.start_time());
	Assert(view.duration() == duration);
	return view;
}


void node_input::end_read_frame() {
	time_unit duration = (this_node().current_time() < past_window_) ? 0 : 1;
	connected_output_->end_read(duration);
}


void node_input::cancel_read_frame() {
	connected_output_->end_read(0);
}


void node_input::set_activated(bool activated) {
	if(activated_ != activated) {
		activated_ = activated;
		if(activated) connected_node().propagate_reconnecting_state();
		else connected_node().propagate_offline_state();
	}
}


bool node_input::has_output_parameter(parameter_id id) const {
	Assert(is_connected());
	return connected_output_->has_output_parameter(id);
}


std::size_t node_input::output_parameters_count() const {
	Assert(is_connected());
	return connected_output_->output_parameters_count();
}


parameter_id node_input::output_parameter_at(std::ptrdiff_t i) const {
	Assert(is_connected());
	return connected_output_->output_parameter_at(i);
}

}}
