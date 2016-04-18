#ifndef MF_TEST_FLOW_H_
#define MF_TEST_FLOW_H_

#include <vector>
#include <functional>
#include <stdexcept>
#include "ndarray.h"
#include "../../src/common.h"
#include "../../src/flow/node.h"
#include "../../src/flow/sink_node.h"
#include "../../src/flow/source_node.h"
#include "../../src/flow/node_input.h"
#include "../../src/flow/node_output.h"
#include "../../src/ndarray/ndcoord.h"
#include "../../src/utility/string.h"
#include "ndarray.h"

namespace mf { namespace test {

constexpr int noframe = -2;


class sequence_frame_source : public flow::source_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;

public:
	output_type<2, int> output;
	
	explicit sequence_frame_source(time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		flow::source_node(seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape), output(*this) { }
	
	void setup() override {
		std::cout << "-------------"<< std::endl;
		output.define_frame_shape(frame_shape_);
	}
	
	void process() override {
		std::cout << "source: t="  << current_time() << std::endl;
		output.view() = make_frame(frame_shape_, current_time());
	}
	
	bool reached_end() const override {
		return (current_time() == last_frame_);
	}
};

// node active when 1+ output active
// active node always produces output to all its outputs,
// even if input(s) desactivated

// node:   active/inactive
// input:  activated/desactivated
// output: active/inactive (still pulls/skips)

class passthrough_node : public flow::node {
public:
	using callback_func = void(passthrough_node& self, input_type<2, int>& in, output_type<2, int>& out);
	
private:
	std::function<callback_func> callback_;
		
	void setup() override {
		output.define_frame_shape(input.frame_shape());	
	}
	
	void pre_process() override {
		if(time_ < activation.size()) input.set_activated(activation[time_]);
	}
	
	void process() override {
		if(callback_) callback_(*this, input, output);
		if(input.view_is_available()) {
			if(frame_index(input.view()) == -1) throw std::runtime_error("invalid frame received in passthrough");
			output.view() = input.view();
		} else {
			output.view() = make_frame(input.frame_shape(), noframe);
		}
	}
	
public:
	input_type<2, int> input;
	output_type<2, int> output;
	
	std::vector<bool> activation;

	passthrough_node(time_unit past_window, time_unit future_window, time_unit prefetch = 0) :
		flow::node(prefetch),
		input(*this, past_window, future_window),
		output(*this) { }

	template<typename Function>
	void set_callback(Function func) {
		callback_ = func;
	}
};



class expected_frames_sink : public flow::sink_node {
private:
	std::vector<int> got_frames_;
	
public:
	std::vector<int> expected_frames;
	input_type<2, int> input;

	std::vector<bool> activation;
	
	explicit expected_frames_sink(const std::vector<int>& seq) :
		expected_frames(seq), input(*this) { }
	
	void pre_process() override {
		if(time_ < activation.size()) {
			input.set_activated(activation[time_]);
		}
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



class input_synchronize_test_node : public flow::node {
private:
	bool failed_ = false;

public:
	input_type<2, int> input1;
	input_type<2, int> input2;
	output_type<2, int> output;
	
	std::vector<bool> activation1;
	std::vector<bool> activation2;
	
	input_synchronize_test_node(time_unit prefetch = 0) :
		flow::node(prefetch),
		input1(*this), input2(*this), output(*this) { }

	void setup() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void pre_process() override {
		if(time_ < activation1.size()) input1.set_activated(activation1[time_]);
		if(time_ < activation2.size()) input2.set_activated(activation2[time_]);
	}
	
	void process() override {
		std::cout << "time " << time_ << std::endl;
		std::cout << "1: av:" << input1.view_is_available() << ", act:" << input1.is_activated() << std::endl;
		std::cout << "2: av:" << input2.view_is_available() << ", act:" << input2.is_activated() << std::endl;
		std::cout << "////////////" << std::endl;
		
		if(input1.view_is_available() && frame_index(input1.view()) == -1) throw std::runtime_error("invalid frame received in merge (input 1)");
		if(input2.view_is_available() && frame_index(input2.view()) == -1) throw std::runtime_error("invalid frame received in merge (input 2)");

		if(input1.view_is_available() && input2.view_is_available()) {
			if(input1.view() != input2.view()) {
				std::cout << "////////////////////////////////////// " << frame_index(input1.view()) << " <--> " << frame_index(input2.view()) << std::endl;
				failed_ = true;
			}
			output.view() = input1.view();
		} else if(input1.view_is_available()) {
			output.view() = input1.view();
		} else if(input2.view_is_available()) {
			output.view() = input2.view();
		} else {
			output.view() = make_frame(input1.frame_shape(), noframe);
		}
	}
	
	bool failed() const { return failed_; }
};


class multiplexer_node : public flow::node {
public:
	input_type<2, int> input;
	output_type<2, int> output1;
	output_type<2, int> output2;
	
	multiplexer_node(time_unit prefetch = 0):
		flow::node(prefetch),
		input(*this), output1(*this), output2(*this) { }
	
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
