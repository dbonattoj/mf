#ifndef MF_DEPTH_CONVERTER_NODE_H_
#define MF_DEPTH_CONVERTER_NODE_H_

#include <algorithm>
#include "../graph/media_node.h"
#include "../graph/media_node_input.h"
#include "../graph/media_node_output.h"
#include "../color.h"
#include "../eigen.h"

namespace mf { namespace node {

class depth_converter : public media_node {	
public:
	output_type<2, Eigen_scalar> output;
	input_type<2, mono_color> input;
	
	Eigen_scalar z_far;
	Eigen_scalar z_near;

	depth_converter() :
		output(*this), input(*this) { }
	
	void setup_() override {
		output.define_frame_shape(input.frame_shape());
	}
	
	void process_() override;
};

}}

#endif
