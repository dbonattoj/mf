#ifndef MF_TEST_FLOW_H_
#define MF_TEST_FLOW_H_

#include <vector>
#include <functional>
#include <stdexcept>
#include "ndarray.h"
#include "../../src/common.h"
#include "../../src/flow/graph.h"
#include "../../src/flow/sink_node.h"
#include "../../src/flow/async_node.h"
#include "../../src/ndarray/ndcoord.h"
#include "../../src/utility/string.h"
#include "ndarray.h"

namespace mf { namespace test {


constexpr int noframe = -2;

class sequence_frame_source : public flow::async_source_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;
	std::set<int> produced_frames_;

public:
	output_type<2, int> output;
	
	explicit sequence_frame_source(time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		flow::async_source_node(seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape),
		output(*this) { name = "source"; }
	
	void setup() override {
		output.define_frame_shape(frame_shape_);
	}
	
	void process() override {
		time_unit t = current_time();
		produced_frames_.emplace(t);
		output.view() = make_frame(frame_shape_, current_time());
	}
	
	bool reached_end() const noexcept override {
		return (current_time() == last_frame_);
	}
	
	bool has_produced_frame(int i) const {
		return (produced_frames_.find(i) != produced_frames_.end());
	}
};


class passthrough_node : public flow::async_node {
private:
	void setup() override {
		output.define_frame_shape(input.frame_shape());	
	}
	
	void pre_process() override {
		if(current_time() < activation.size()) input.set_activated(activation[current_time()]);
	}
	
	void process() override {
		if(callback) callback(*this, input, output);
		if(input.view_is_available()) {
			output.view() = input.view();
		} else {
			output.view() = make_frame(input.frame_shape(), noframe);
		}
	}
	
public:
	using callback_func = void(passthrough_node& self, input_type<2, int>& in, output_type<2, int>& out);

	input_type<2, int> input;
	output_type<2, int> output;
	
	std::vector<bool> activation;
	std::function<callback_func> callback;

	passthrough_node(time_unit past_window, time_unit future_window, time_unit prefetch = 0) :
		input(*this, past_window, future_window),
		output(*this) { name = "passthrough"; }
};



class expected_frames_sink : public flow::sink_node {
private:
	std::vector<int> got_frames_;
	
public:
	std::vector<int> expected_frames;
	input_type<2, int> input;

	std::vector<bool> activation;
	
	explicit expected_frames_sink(const std::vector<int>& seq) :
		expected_frames(seq), input(*this) { name = "sink"; }
	
	void pre_process() override {
		if(current_time() < activation.size())
			input.set_activated(activation[current_time()]);
	}
	
	void process() override {
		int index;
		if(input.view_is_available()) index = frame_index(input.view());
		else index = noframe;
		got_frames_.push_back(index);
	}

	bool check() const {
		bool ok = (expected_frames == got_frames_);
		if(! ok) print();
		return ok;
	}
	
	void print() const {
		std::cout << "expected: {" << to_string(expected_frames.begin(), expected_frames.end()) << "}\n"
		          << "     got: {" << to_string(got_frames_.begin(), got_frames_.end()) << "}" << std::endl;
	}
};



class input_synchronize_test_node : public flow::async_node {
public:
	input_type<2, int> input1;
	input_type<2, int> input2;
	output_type<2, int> output;
	
	std::vector<bool> activation1;
	std::vector<bool> activation2;
	
	input_synchronize_test_node(time_unit prefetch = 0) :
		input1(*this), input2(*this), output(*this) { name = "merge"; }


	void setup() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void pre_process() override {
		time_unit t = current_time();
		if(t < activation1.size()) input1.set_activated(activation1[t]);
		if(t < activation2.size()) input2.set_activated(activation2[t]);
	}
	
	void process() override {
		int iout = noframe;
		REQUIRE(input1.view_is_available() == input1.is_activated());
		REQUIRE(input2.view_is_available() == input2.is_activated());
		if(input1.is_activated() && input2.is_activated()) {
			int i1 = frame_index(input1.view());
			int i2 = frame_index(input2.view());
			
			REQUIRE(i1 != -1);
			REQUIRE(i2 != -1);
			
			if(i1 != noframe) iout = i1;
			else if(i2 != noframe) iout = i2;
			
			if(i1 != noframe && i2 != noframe) REQUIRE(i1 == i2);
		} else if(input1.is_activated()) {
			iout = frame_index(input1.view());
		} else if(input2.is_activated()) {
			iout = frame_index(input2.view());
		}
		output.view() = make_frame(input1.frame_shape(), iout);
	}
};


class multiplexer_node : public flow::async_node {
public:
	input_type<2, int> input;
	output_type<2, int> output1;
	output_type<2, int> output2;
	
	multiplexer_node(time_unit prefetch = 0):
		input(*this), output1(*this), output2(*this) { name = "multiplex"; }
	
	void setup() override {
		output1.define_frame_shape(input.frame_shape());
		output2.define_frame_shape(input.frame_shape());
	}
	
	void process() override {
		if(frame_index(input.view()) == -1) throw std::runtime_error("invalid frame received in multiplexer");	
		output1.view() = input.view();
		output2.view() = input.view();
	}
};


}}

#endif
