#ifndef MF_BLEND_CLOSEST_NODE_H_
#define MF_BLEND_CLOSEST_NODE_H_

#include "../../src/flow/async_node.h"
#include "../../src/../camera/projection_image_camera.h"
#include "../../src/../color.h"
#include <memory>

namespace mf { namespace node {

class blend_closest_node : public flow::async_node {
public:
	using camera_type = projection_image_camera<std::uint8_t>;

	struct input_visual {
		input_prop(blend_closest_node& self, const camera_type& cam) :
			image_in(self), depth_in(self), camera(cm) { }
		
		input_type<2, rgba_color> image_in;
		input_type<2, std::uint8_t> depth_in;
		camera_type camera;
	};
	
	struct active_input_visual {
		active_input_visual(input_visual* vis, real dist) :
			visual(vis), camera_distance(dist) { }
		
		input_visual* visual;
		real camera_distance;
	}

private:
	std::size_t number_of_active_inputs_;
	camera_type output_camera_;
	std::vector<std::unique_ptr<input_visual>> visuals_;
	
	std::vector<active_input_visual> active_visuals_;

public:
	output_type<2, rgba_color> out;
		
	explicit blend_closest_node(const camera_type& cam_out, std::size_t n = 3) :
		number_of_active_inputs_(n), output_camera_(cam_out), out(*this) { }

	input_visual& add_input_visual(const camera_type& cam) {
		visuals_.emplace_back(new input_prop(*this, cam));
		return *ins.back();
	}

protected:
	void setup() override;
	void pre_process() override;
	void process() override;
};

}}

#endif
