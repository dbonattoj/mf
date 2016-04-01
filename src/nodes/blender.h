#ifndef MF_NODE_BLENDER_H_
#define MF_NODE_BLENDER_H_

#include <algorithm>
#include "../graph/media_node.h"
#include "../graph/media_node_input.h"
#include "../graph/media_node_output.h"
#include "../color.h"

namespace mf { namespace node {

template<typename Color>
class blender : public media_node {	
public:
	output_type<2, Color> output;
	input_type<2, Color> input1;
	input_type<2, Color> input2;

	blender() :
		output(*this), input1(*this), input2(*this) { }
	
	void setup_() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void process_() override {
		std::transform(
			input1.view().begin(),
			input1.view().end(),
			input2.view().begin(),
			output.view().begin(),
			[](const rgb_color& col1, const rgb_color& col2) -> rgb_color {
				rgb_color col;
				if(col1.r == 0 && col1.g == 0 && col1.b == 0) return col2;
				if(col2.r == 0 && col2.g == 0 && col2.b == 0) return col1;
				col.r = col1.r/2 + col2.r/2;
				col.g = col1.g/2 + col2.g/2;
				col.b = col1.b/2 + col2.b/2;
				return col;
			}
		);
	}
};

}}

#endif
