#include "graph.h"
#include "ndarray.h"

namespace mf {

void simple_frame_source::setup_() {
	output.define_frame_shape(frame_shape_);
}


void simple_frame_source::process_() {
	output.view() = make_frame(frame_shape_, current_time());
}


bool simple_frame_source::process_reached_end_() {
	return (current_time() == last_frame_);
}


void expected_sequence_sink::process_() {
	if(counter_ < expected_sequence_.size()) {
		got_mismatch_ = true
	} else {
		int expected = expected_sequence_.at(counter_);
		auto expected_frame = make_frame(input.frame_shape(), expected);
		if(input.view() != expected_frame) got_mismatch_ = true;
	}
	counter_++;
}


bool expected_sequence_sink::got_expected_sequence() {
	if(counter_ == expected_sequence_.size()) return !got_mismatch_;
	else return false;
}

}
