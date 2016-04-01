#ifndef MF_WARP_NODE_H_
#define MF_WARP_NODE_H_

#include "../graph/media_node.h"
#include "../graph/media_node_input.h"
#include "../graph/media_node_output.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"
#include "../eigen.h"

namespace mf { namespace node {
	
template<typename Color>
class warp : public media_node {
public:
	input_type<2, Color> image_input;
	input_type<2, Eigen_scalar> depth_input;
	output_type<2, Color> output;
	
	Color background_color{50,50,50};
	
	projection_image_camera* input_camera;
	projection_image_camera* output_camera;
	
	warp() :
		image_input(*this), depth_input(*this), output(*this) { }

protected:
	void setup_() override;
	void process_() override;
};

}}

#include "warp.tcc"

#endif
