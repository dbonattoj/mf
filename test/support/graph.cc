#include "graph.h"
#include "ndarray.h"
#include <iostream>

namespace mf { namespace test {

void sequence_frame_source::setup_() {
	output.define_frame_shape(frame_shape_);
}


void sequence_frame_source::process_() {
	output.view() = make_frame(frame_shape_, current_time());
}


bool sequence_frame_source::process_reached_end_() {
	return (current_time() == last_frame_);
}


void expected_frames_sink::process_() {
	if(counter_ >= expected_frames_.size()) {
		got_mismatch_ = true;
	} else {
		int expected = expected_frames_.at(counter_);
		auto expected_frame = make_frame(input.frame_shape(), expected);
		if(input.view() != expected_frame) got_mismatch_ = true;
	}
	counter_++;
}


bool expected_frames_sink::got_expected_frames() const {
	if(counter_ == expected_frames_.size()) return !got_mismatch_;
	else return false;
}


void passthrough_node::setup_() {
	output.define_frame_shape(input.frame_shape());
}


passthrough_node::passthrough_node(time_unit past_window, time_unit future_window) :
	input(*this, past_window, future_window),
	output(*this) { }



void passthrough_node::process_() {
	if(callback_) callback_(*this, input, output);
	output.view() = input.view();
}


}}
