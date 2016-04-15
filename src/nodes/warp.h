#ifndef MF_WARP_NODE_H_
#define MF_WARP_NODE_H_

#include "../flow/node.h"
#include "../flow/node_input.h"
#include "../flow/node_output.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf { namespace node {
	
template<typename Color, typename Depth>
class warp : public flow::node {
public:
	input_type<2, Color> image_input;
	input_type<2, mono_color> depth_input;
	output_type<2, Color> output;
	
	Color background_color{0,0,0};
	
	projection_image_camera<Depth>* input_camera;
	projection_image_camera<Depth>* output_camera;
	
	warp() :
		image_input(*this), depth_input(*this), output(*this) { }

protected:
	void setup() override;
	void process() override;
};

}}

#include "warp.tcc"

#endif
