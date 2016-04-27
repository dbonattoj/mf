#ifndef MF_COLOR_CONVERTER_NODE_H_
#define MF_COLOR_CONVERTER_NODE_H_

#include <algorithm>
#include "../flow/thin_node.h"
#include "../color.h"

namespace mf { namespace node {

template<typename Input_color, typename Output_color>
class color_converter : public flow::thin_node<2, Input_color, Output_color> {	
public:
	output_type<2, Output_color> out;
	input_type<2, Input_color> in;

	color_converter() :
		out(*this), in(*this) { }
	
	void setup() override {
		out.define_frame_shape(in.frame_shape());
	}
	
	void process() override {
		std::transform(
			in.view().begin(),
			in.view().end(),
			out.view().begin(),
			color_convert<Output_color, Input_color>
		);
	}
};

}}

#endif
