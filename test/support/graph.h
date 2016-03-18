#ifndef MF_TEST_GRAPH_H_
#define MF_TEST_GRAPH_H_

#include <vector>
#include <functional>
#include "ndarray.h"
#include "../../src/common.h"
#include "../../src/graph/media_sequential_node.h"
#include "../../src/graph/media_sink_node.h"
#include "../../src/graph/media_node_input.h"
#include "../../src/graph/media_node_output.h"
#include "../../src/ndarray/ndcoord.h"

namespace mf { namespace test {

class sequence_frame_source : public media_sequential_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;

public:
	media_node_output<2, int> output;

	explicit sequence_frame_source(time_unit last_frame, const ndsize<2>& frame_shape) :
		last_frame_(last_frame), frame_shape_(frame_shape), output(*this) { }
	
	void setup_() override {
		output.define_frame_shape(frame_shape_);
	}
	
	void process_() override {
		output.view() = make_frame(frame_shape_, current_time());
	}
	
	bool process_reached_end_() override {
		return (current_time() == last_frame_);
	}
};


class expected_frames_sink : public media_sink_node {
private:
	const std::vector<int> expected_frames_;
	
	bool got_mismatch_ = false;
	std::size_t counter_ = 0;

public:
	media_node_input<2, int> input;
	
	explicit expected_frames_sink(const std::vector<int>& seq) :
		expected_frames_(seq), input(*this) { }
	
	void process_() override {
		if(counter_ >= expected_frames_.size()) {
			got_mismatch_ = true;
		} else {
			int expected = expected_frames_.at(counter_);
			auto expected_frame = make_frame(input.frame_shape(), expected);
			if(input.view() != expected_frame) got_mismatch_ = true;
		}
		counter_++;
	}
	
	bool got_expected_frames() const {
		if(counter_ == expected_frames_.size()) return !got_mismatch_;
		else return false;	
	}
};


class passthrough_node : public media_sequential_node {
public:
	using input_type = media_node_input<2, int>;
	using output_type = media_node_output<2, int>;
	using callback_func = void(passthrough_node& self, input_type& in, output_type& out);
	
private:
	std::function<callback_func> callback_;
	
	void setup_() override {
		output.define_frame_shape(input.frame_shape());	
	}
	
	void process_() override {
		if(callback_) callback_(*this, input, output);
		output.view() = input.view();
	}
	
public:
	input_type input;
	output_type output;

	passthrough_node(time_unit past_window, time_unit future_window) :
		input(*this, past_window, future_window),
		output(*this) { }

	template<typename Function>
	void set_callback(Function func) {
		callback_ = func;
	}
};


class input_synchronize_test_node : public media_sequential_node {
private:
	bool failed_ = false;

public:
	media_node_input<2, int> input1;
	media_node_input<2, int> input2;
	media_node_output<2, int> output;
	
	input_synchronize_test_node() :
		input1(*this), input2(*this), output(*this) { }

	void setup_() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void process_() override {		
		if(input1.view() != input2.view()) failed_ = true;
		output.view() = input1.view();
	}
	
	bool failed() const { return failed_; }
};


class multiplexer_node : public media_sequential_node {
public:
	media_node_input<2, int> input;
	media_node_output<2, int> output1;
	media_node_output<2, int> output2;
	
	multiplexer_node():
		input(*this), output1(*this), output2(*this) { }
	
	void setup_() override {
		output1.define_frame_shape(input.frame_shape());
		output2.define_frame_shape(input.frame_shape());
	}
	
	void process_() override {		
		output1.view() = input.view();
		output2.view() = input.view();
	}
};


}}

#endif
