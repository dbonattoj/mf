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

#ifndef MF_TEST_FLOW_H_
#define MF_TEST_FLOW_H_

#include <vector>
#include <functional>
#include <stdexcept>
#include "ndarray.h"
#include <mf/common.h>
#include <mf/filter/filter_graph.h>
#include <mf/filter/filter.h>
#include <mf/nd/ndcoord.h>
#include <mf/utility/string.h>
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
	
	time_unit previous_frame_ = -1;
	bool seeked_back_ = false;

public:
	output_type<2, int> output;
	
	explicit sequence_frame_source(time_unit last_frame, const ndsize<2>& frame_shape, bool seekable, bool bounded = false) :
		flow::source_filter(seekable, (bounded || seekable) ? (last_frame + 1) : -1), last_frame_(last_frame), frame_shape_(frame_shape),
		output(*this) { set_name("source"); }
	
	void setup() override {
		std::cout << "source setup" << std::endl;

		output.define_frame_shape(frame_shape_);
	}
	
	void process(flow::filter_job& job) override {
		time_unit t = job.time();

		if(previous_frame_ != -1 && t < previous_frame_)
			seeked_back_ = true;

		produced_frames_.emplace(t);
		job.out(output) = make_frame(frame_shape_, t);
		if(t == last_frame_) job.mark_end();
		
		previous_frame_ = t;
		
		//std::cout << "       >> " << t << '\n';
	}
	
	bool has_produced_frame(int i) const {
		return (produced_frames_.find(i) != produced_frames_.end());
	}
	
	bool seeked_back() const {
		return seeked_back_;
	}
};


///////////////


class passthrough_filter : public flow::filter {
private:
	void setup() override {
		output.define_frame_shape(input.frame_shape());	
	}
	
	void pre_process(flow::filter_job& job) override {
		time_unit t = job.time();
		if(t < activation.size()) input.set_activated(activation[t]);
	}
	
	void process(flow::filter_job& job) override {		
		time_unit t = job.time();
		
		auto in = job.in(input);
		auto in_full = job.in_full(input);
		auto out = job.out(output);
	
		if(callback) callback(*this, job);

		if(in) {
			MF_TEST_THREAD_REQUIRE(! in_full.is_null());
			for(time_unit u = in_full.span().start_time(); u < in_full.span().end_time(); ++u)
				MF_TEST_THREAD_REQUIRE(frame_index(in_full[u - in_full.span().start_time()]) == u);
			out = in;
		} else {
			out = make_frame(input.frame_shape(), noframe);
		}
	}
	
public:
	using callback_func = void(passthrough_filter& self, flow::filter_job& job);

	input_type<2, int> input;
	output_type<2, int> output;
	
	std::vector<bool> activation;
	std::function<callback_func> callback;

	passthrough_filter(time_unit past_window, time_unit future_window) :
		input(*this, past_window, future_window),
		output(*this) { set_name("passthrough"); }
};


///////////////


class to_float_filter : public flow::filter {
private:
	void setup() override {
		output.define_frame_shape(input.frame_shape());	
	}
		
	void process(flow::filter_job& job) override {				
		auto in = job.in(input);
		auto out = job.out(output);
		out = in;
	}
	
public:
	input_type<2, int> input;
	output_type<2, float> output;
	
	to_float_filter() :
		input(*this, 0, 0),
		output(*this) { set_name("to float"); }
};


inline int to_int(float f) {
	return static_cast<int>(f);
}

///////////////


class multiple_output_filter : public flow::filter {
private:
	void setup() override {
		std::cout << "multiout setup" << std::endl;

		output1.define_frame_shape(input.frame_shape());	
		output2.define_frame_shape(input.frame_shape());	
	}
	
	void process(flow::filter_job& job) override {				
		auto in = job.in(input);
		auto out1 = job.out(output1);
		auto out2 = job.out(output2);
		
		out1 = in;
		out2 = in;
	}

public:
	input_type<2, int> input;
	output_type<2, int> output1;
	output_type<2, int> output2;
	
	multiple_output_filter() :
		input(*this), output1(*this), output2(*this) { set_name("multiout"); }
};


///////////////


class simple_sink : public flow::sink_filter {
public:
	input_type<2, int> input;
	
	simple_sink() :
		input(*this) { }
	
	void process(flow::filter_job& job) override { set_name("sink"); }
};


///////////////



class expected_frames_sink : public flow::sink_filter {
private:
	const std::vector<int> expected_frames_;
	std::vector<int> got_frames_;
	
public:
	input_type<2, int> input;

	std::vector<bool> activation;
	
	explicit expected_frames_sink(const std::vector<int>& seq) :
		expected_frames_(seq),
		got_frames_(seq.size(), missingframe),
		input(*this) { set_name("sink"); }
	
	void pre_process(flow::filter_job& job) override {
		time_unit t = job.time();
		if(t < activation.size())
			input.set_activated(activation[t]);
	}
	
	void process(flow::filter_job& job) override {		
		auto in = job.in(input);
		
		int index;
		if(in) index = frame_index(in);
		else index = noframe;
		
		time_unit t = job.time();
		while(t >= got_frames_.size()) got_frames_.push_back(missingframe);
		
		got_frames_[t] = index;
		if(!(index == expected_frames_.at(t))) print();
		MF_TEST_THREAD_REQUIRE(index == expected_frames_.at(t));
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


///////////////


class input_synchronize_test_filter : public flow::filter {
public:
	input_type<2, int> input1;
	input_type<2, int> input2;
	output_type<2, int> output;
	
	std::vector<bool> activation1;
	std::vector<bool> activation2;
	
	input_synchronize_test_filter() :
		input1(*this), input2(*this), output(*this) { set_name("merge"); }


	void setup() override {
		std::cout << "merge setup" << std::endl;
		output.define_frame_shape(input1.frame_shape());
	}
	
	void pre_process(flow::filter_job& job) override {
		time_unit t = job.time();
		if(t < activation1.size()) input1.set_activated(activation1[t]);
		if(t < activation2.size()) input2.set_activated(activation2[t]);
	}
	
	void process(flow::filter_job& job) override {
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
