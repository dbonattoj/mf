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
