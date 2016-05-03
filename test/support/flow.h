#ifndef MF_TEST_FLOW_H_
#define MF_TEST_FLOW_H_

#include <vector>
#include <functional>
#include <stdexcept>
#include "ndarray.h"
#include "../../src/common.h"
#include "../../src/flow/graph.h"
#include "../../src/flow/node.h"
#include "../../src/flow/sink_node.h"
#include "../../src/flow/async_node.h"
#include "../../src/ndarray/ndcoord.h"
#include "../../src/utility/string.h"
#include "ndarray.h"
#include "thread.h"

namespace mf { namespace test {


constexpr int noframe = -2;
constexpr int missingframe = -3;

class sequence_frame_source : public flow::async_source_node {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;
	std::set<int> produced_frames_;

public:
	output_type<2, int> output;
	
	explicit sequence_frame_source(flow::graph& gr, time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		flow::async_source_node(gr, seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape),
		output(*this) { name = "source"; }
	
	void setup() override {
		output.define_frame_shape(frame_shape_);
	}
	
	void process(flow::node_job& job) override {
		if(! output.is_active()) return;
		time_unit t = job.time();
		produced_frames_.emplace(t);
		job.out(output) = make_frame(frame_shape_, current_time());
		if(t == last_frame_) job.mark_end();
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
	
	void pre_process(time_unit t) override {
		if(t < activation.size()) input.set_activated(activation[t]);
	}
	
	void process(flow::node_job& job) override {
		auto out = job.out(output);
		auto in = job.in(input);
		
		if(callback) callback(*this, in, out);
		if(in) out = in;
		else out = make_frame(in.shape(), noframe);
	}
	
public:
	using callback_func = void(passthrough_node& self, ndarray_view<2, int>& in, ndarray_view<2, int>& out);

	input_type<2, int> input;
	output_type<2, int> output;
	
	std::vector<bool> activation;
	std::function<callback_func> callback;

	passthrough_node(flow::graph& gr, time_unit past_window, time_unit future_window, time_unit prefetch = 0) :
		flow::async_node(gr),
		input(*this, past_window, future_window),
		output(*this) { name = "passthrough"; }
};



class expected_frames_sink : public flow::sink_node {
private:
	const std::vector<int> expected_frames_;
	std::vector<int> got_frames_;
	
public:
	input_type<2, int> input;

	std::vector<bool> activation;
	
	explicit expected_frames_sink(flow::graph& gr, const std::vector<int>& seq) :
		flow::sink_node(gr),
		expected_frames_(seq),
		got_frames_(seq.size(), missingframe),
		input(*this)
	{
		name = "sink";
	}
	
	void pre_process(time_unit t) override {
		if(t < activation.size())
			input.set_activated(activation[t]);
	}
	
	void process(flow::node_job& job) override {
		auto in = job.in(input);
		
		int index;
		if(in) index = frame_index(in);
		else index = noframe;
		
		time_unit t = current_time();
		while(t >= got_frames_.size()) got_frames_.push_back(missingframe);
		
		got_frames_[t] = index;
	}

	bool check() const {
		bool ok = (expected_frames_ == got_frames_);
		if(! ok) print();
		return ok;
	}
	
	void print() const {
		std::cout << "expected: {" << to_string(expected_frames_.begin(), expected_frames_.end()) << "}\n"
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
	
	input_synchronize_test_node(flow::graph& gr, time_unit prefetch = 0) :
		flow::async_node(gr),
		input1(*this),
		input2(*this),
		output(*this) { name = "merge"; }


	void setup() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void pre_process(time_unit t) override {
		if(t < activation1.size()) input1.set_activated(activation1[t]);
		if(t < activation2.size()) input2.set_activated(activation2[t]);
	}
	
	void process(flow::node_job& job) override {
		auto in1 = job.in(input1);
		auto in2 = job.in(input2);
		auto out = job.out(output);

		int iout = noframe;
		if(in1 && in2) {
			int i1 = frame_index(in1);
			int i2 = frame_index(in2);
			
			MF_TEST_THREAD_REQUIRE(i1 != -1);
			MF_TEST_THREAD_REQUIRE(i2 != -1);
			
			if(i1 != noframe) iout = i1;
			else if(i2 != noframe) iout = i2;
			
			if(i1 != noframe && i2 != noframe) MF_TEST_THREAD_REQUIRE(i1 == i2);
		} else if(in1) {
			iout = frame_index(in1);
		} else if(in2) {
			iout = frame_index(in2);
		}
		out = make_frame(in1.shape(), iout);
	}
};


}}

#endif
