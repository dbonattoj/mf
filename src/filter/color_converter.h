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

#ifndef MF_COLOR_CONVERTER_FILTER_H_
#define MF_COLOR_CONVERTER_FILTER_H_

#include <algorithm>
#include "filter.h"
#include "../color.h"

namespace mf { namespace flow {

/// Color converter filter.
/** Runs \ref color_convert function on each elem. */
template<typename Input_color, typename Output_color>
class color_converter_filter : public filter {
public:
	output_type<2, Output_color> output;
	input_type<2, Input_color> input;

	color_converter_filter(filter_node& nd) :
		filter(nd), output(*this), input(*this) { }
	
	void setup() override {
		output.define_frame_shape(input.frame_shape());
	}
	
	void process(node_job& job) override {
		auto in = job.in(input);
		auto out = job.out(output);
		std::transform(
			in.begin(),
			in.end(),
			out.begin(),
			color_convert<Output_color, Input_color>
		);
	}
};

// TODO insert automatically, make thin_node

}}

#endif
