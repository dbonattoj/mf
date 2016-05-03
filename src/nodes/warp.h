#ifndef MF_WARP_NODE_H_
#define MF_WARP_NODE_H_

#include "../flow/async_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf { namespace node {
	
template<typename Color, typename Depth>
class warp : public flow::async_node {
public:
	using camera_type = projection_image_camera<Depth>;

private:
	camera_type input_camera_;
	camera_type output_camera_;

public:
	input_type<2, Color> image_input;
	input_type<2, Depth> depth_input;
	output_type<2, Color> output;
		
	warp(const camera_type& cam_in, const camera_type& cam_out, const Color& background = Color::black) :
		input_camera_(cam_in), output_camera_(cam_out),
		image_input(*this), depth_input(*this), output(*this) { }

protected:
	void setup() override;
	void process(node_job&) override;
};

}}

#include "warp.tcc"

#endif
