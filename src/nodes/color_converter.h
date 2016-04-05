#ifndef MF_COLOR_CONVERTER_NODE_H_
#define MF_COLOR_CONVERTER_NODE_H_

#include <algorithm>
#include "../graph/media_node.h"
#include "../graph/media_node_input.h"
#include "../graph/media_node_output.h"
#include "../color.h"

namespace mf { namespace node {

template<typename Input_color, typename Output_color>
class color_converter : public media_node {	
public:
	output_type<2, Output_color> output;
	input_type<2, Input_color> input;

	color_converter() :
		output(*this), input(*this) { }
	
	void setup_() override {
		output.define_frame_shape(input.frame_shape());
	}
	
	void process_() override {
		std::transform(
			input.view().begin(),
			input.view().end(),
			output.view().begin(),
			color_convert<Output_color, Input_color>
		);
	}
};

}}

#endif