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
	Color background_color_;

public:
	input_type<2, Color> image_in;
	input_type<2, Depth> depth_in;
	output_type<2, Color> out;
		
	warp(const camera_type& cam_in, const camera_type& cam_out, const Color& background = Color::black) :
		input_camera_(cam_in), output_camera_(cam_out), background_color_(background),
		image_in(*this), depth_in(*this), out(*this) { }

protected:
	void setup() override;
	void process() override;
};

}}

#include "warp.tcc"

#endif
