#ifndef MF_BLEND_CLOSEST_NODE_H_
#define MF_BLEND_CLOSEST_NODE_H_

#include "../../src/flow/sync_node.h"
#include "../../src/camera/projection_image_camera.h"
#include "../../src/color.h"
#include <memory>

namespace mf { namespace node {

class blend_closest_node : public flow::sync_node {
public:
	using camera_type = projection_image_camera<std::uint8_t>;

	struct input_visual {
		input_visual(blend_closest_node& self, const camera_type& cam) :
			image_input(self), camera(cam) { }
		
		input_type<2, rgba_color> image_input;
		camera_type camera;
	};
	
	struct active_input_visual {
		active_input_visual(input_visual* vis, real dist) :
			visual(vis), camera_distance(dist) { }
		
		input_visual* visual;
		real camera_distance;
	};

private:
	std::size_t number_of_active_inputs_;
	camera_type output_camera_;
	std::vector<std::unique_ptr<input_visual>> visuals_;
	
	std::vector<active_input_visual> active_visuals_;

public:
	output_type<2, rgba_color> output;
		
	blend_closest_node(flow::graph& gr, const camera_type& cam_out, std::size_t n = 3) :
		flow::sync_node(gr), number_of_active_inputs_(n), output_camera_(cam_out), output(*this) { }

	input_visual& add_input_visual(const camera_type& cam) {
		visuals_.emplace_back(new input_visual(*this, cam));
		return *visuals_.back();
	}

protected:
	void setup() override;
	void pre_process(time_unit t) override;
	void process(flow::node_job&) override;
};

}}

#endif
