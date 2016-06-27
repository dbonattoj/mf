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

#ifndef MF_FLOW_SIMPLE_FILTER_H_
#define MF_FLOW_SIMPLE_FILTER_H_

#include "filter.h"

namespace mf { namespace flow {

/// Filter with most basic configuration, base class
/** Takes one input and one output, or same dimension. Takes no time window on input. Does not deactivate the input.
 ** Sets output to same frame shape as input. */
template<std::size_t Dim, typename Input_elem, typename Output_elem>
class simple_filter : public filter {
public:
	input_type<Dim, Input_elem> input;
	output_type<Dim, Output_elem> output;

	using input_view_type = ndarray_view<Dim, const Input_elem>;
	using output_view_type = ndarray_view<Dim, Output_elem>;

protected:
	virtual void process_frame(const input_view_type& in, const output_view_type& out, filter_job& job) = 0;

public:
	simple_filter(filter_node& nd) :
		filter(nd), input(*this), output(*this) { }

	void setup() final override {
		output.define_frame_shape(input.frame_shape());
	}
	
	void pre_process(job_type&) final override { }
	
	void process(job_type& job) final override {
		this->process_frame(job.in(input), job.out(output), job);
	}
};

}}

#endif
