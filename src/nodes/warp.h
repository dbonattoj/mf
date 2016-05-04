#ifndef MF_WARP_NODE_H_
#define MF_WARP_NODE_H_

#include "../flow/sync_node.h"
#include "../camera/projection_image_camera.h"
#include "../color.h"

namespace mf { namespace node {
	
template<typename Color, typename Depth>
class warp : public flow::sync_node {
public:
	using camera_type = projection_image_camera<Depth>;

private:
	camera_type input_camera_;
	camera_type output_camera_;

public:
	input_type<2, Color> image_input;
	input_type<2, Depth> depth_input;
	output_type<2, Color> output;
		
	warp(flow::graph& gr, const camera_type& cam_in, const camera_type& cam_out, const Color& background = Color::black) :
		flow::sync_node(gr), input_camera_(cam_in), output_camera_(cam_out),
		image_input(*this), depth_input(*this), output(*this) { }

protected:
	void setup() override;
	void process(flow::node_job&) override;
};

}}

#include "warp.tcc"

#endif
