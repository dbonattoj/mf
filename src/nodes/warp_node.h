#ifndef MF_WARP_NODE_H_
#define MF_WARP_NODE_H_

#include "../graph/media_node.h"
#include "../graph/media_node_input.h"
#include "../graph/media_node_output.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf {

template<typename Color>
class warp_node : public media_node {
public:
	media_node_input<2, Color> image_input;
	media_node_input<2, mono_color> depth_input;
	media_node_output<2, Color> output;
	
	projection_image_camera* input_camera;
	projection_image_camera* output_camera;
	
	warp_node() : image_input(*this), depth_input(*this), output(*this) { }

protected:
	void setup_() override;
	void process_() override;
};

}

#include "warp_node.tcc"

#endif
