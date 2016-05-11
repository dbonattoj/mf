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
#include "../../src/ndarray/ndcoord.h"
#include "../../src/utility/string.h"
#include "ndarray.h"
#include "thread.h"


namespace mf { namespace test {


constexpr int noframe = -2;
constexpr int missingframe = -3;

template<typename Base>
class sequence_frame_source : public Base {
private:
	time_unit last_frame_;
	ndsize<2> frame_shape_;
	std::set<int> produced_frames_;

public:
	typename Base::template output_type<2, int> output;
	
	explicit sequence_frame_source(flow::graph& gr, time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		Base(gr, seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape),
		output(*this) { Base::name = "source"; }
	
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


template<typename Base>
class passthrough_node : public Base {
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
	using callback_func = void(passthrough_node& self, flow::node_job& job);

	typename Base::template input_type<2, int> input;
	typename Base::template output_type<2, int> output;
	
	std::vector<bool> activation;
	std::function<callback_func> callback;

	passthrough_node(flow::graph& gr, time_unit past_window, time_unit future_window) :
		Base(gr),
		input(*this, past_window, future_window),
		output(*this) { Base::name = "passthrough"; }
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



template<typename Base>
class input_synchronize_test_node : public Base {
public:
	typename Base::template input_type<2, int> input1;
	typename Base::template input_type<2, int> input2;
	typename Base::template output_type<2, int> output;
	
	std::vector<bool> activation1;
	std::vector<bool> activation2;
	
	input_synchronize_test_node(flow::graph& gr, time_unit prefetch = 0) :
		Base(gr),
		input1(*this),
		input2(*this),
		output(*this) { Base::name = "merge"; }


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
