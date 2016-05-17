#ifndef MF_TEST_FLOW_H_
#define MF_TEST_FLOW_H_

#include <vector>
#include <functional>
#include <stdexcept>
#include "ndarray.h"
#include "../../src/common.h"
#include "../../src/flow/graph.h"
#include "../../src/filter/filter.h"
#include "../../src/ndarray/ndcoord.h"
#include "../../src/utility/string.h"
#include "ndarray.h"
#include "thread.h"


namespace mf { namespace test {


constexpr int noframe = -2;
constexpr int missingframe = -3;

class sequence_frame_source : public flow::source_filter {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;
	std::set<int> produced_frames_;

public:
	output_type<2, int> output;
	
	explicit sequence_frame_source(flow::filter_node& nd, time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		flow::source_filter(nd, seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape),
		output(*this) { }
	
	void setup() override {
		output.define_frame_shape(frame_shape_);
	}
	
	void process(flow::node_job& job) override {
		time_unit t = job.time();
		produced_frames_.emplace(t);
		job.out(output) = make_frame(frame_shape_, t);
		if(t == last_frame_) job.mark_end();
	}
	
	bool has_produced_frame(int i) const {
		return (produced_frames_.find(i) != produced_frames_.end());
	}
};


class passthrough_filter : public flow::filter {
private:
	void setup() override {
		output.define_frame_shape(input.frame_shape());	
	}
	
	void pre_process(flow::node_job& job) override {
		time_unit t = job.time();
		if(t < activation.size()) input.set_activated(activation[t]);
	}
	
	void process(flow::node_job& job) override {
		auto in = job.in(input);
		auto out = job.out(input);
	
		if(callback) callback(*this, job);
		if(in) out = in;
		else out = make_frame(input.frame_shape(), noframe);
	}
	
public:
	using callback_func = void(passthrough_filter& self, flow::node_job& job);

	input_type<2, int> input;
	output_type<2, int> output;
	
	std::vector<bool> activation;
	std::function<callback_func> callback;

	passthrough_filter(flow::filter_node& nd, time_unit past_window, time_unit future_window) :
		flow::filter(nd),
		input(*this, past_window, future_window),
		output(*this) { }
};



class expected_frames_sink : public flow::sink_filter {
private:
	const std::vector<int> expected_frames_;
	std::vector<int> got_frames_;
	
public:
	input_type<2, int> input;

	std::vector<bool> activation;
	
	explicit expected_frames_sink(flow::filter_node& nd, const std::vector<int>& seq) :
		flow::sink_filter(nd),
		expected_frames_(seq),
		got_frames_(seq.size(), missingframe),
		input(*this) { }
	
	void pre_process(flow::node_job& job) override {
		time_unit t = job.time();
		if(t < activation.size())
			input.set_activated(activation[t]);
	}
	
	void process(flow::node_job& job) override {
		auto in = job.in(input);
		
		int index;
		if(in) index = frame_index(in);
		else index = noframe;
		
		time_unit t = job.time();
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



class input_synchronize_test_filter : public flow::filter {
public:
	input_type<2, int> input1;
	input_type<2, int> input2;
	output_type<2, int> output;
	
	std::vector<bool> activation1;
	std::vector<bool> activation2;
	
	input_synchronize_test_filter(flow::filter_node& nd, time_unit prefetch = 0) :
		flow::filter(nd), input1(*this), input2(*this), output(*this) { }


	void setup() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void pre_process(flow::node_job& job) override {
		time_unit t = job.time();
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
		out = make_frame(input1.frame_shape(), iout);
	}
};


}}

#endif
