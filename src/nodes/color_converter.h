#ifndef MF_COLOR_CONVERTER_NODE_H_
#define MF_COLOR_CONVERTER_NODE_H_

#include <algorithm>
#include "../flow/sync_node.h"
#include "../color.h"

namespace mf { namespace node {

template<typename Input_color, typename Output_color>
class color_converter : public flow::sync_node {	
public:
	output_type<2, Output_color> output;
	input_type<2, Input_color> input;

	color_converter() :
		output(*this), input(*this) { }
	
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

}}

#endif
