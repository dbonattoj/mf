#ifndef PROG_BLEND_CLOSEST_FILTER_H_
#define PROG_BLEND_CLOSEST_FILTER_H_

#include <memory>
#include <mf/filter/filter.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include <mf/masked_elem.h>
#include "../support/common.h"

/// Node blends input visuals with closest camera positions.
/** _Input visual_ is an input port of image type `<2, rgba_color>`, associated with a \ref camera. Multiple input
 ** visuals are added after construction using add_input_visual().
 ** The node selects the _n_ input visuals whose camera is closest to `output_camera`, and does weighed pixel-wise
 ** blending, ignoring null pixels on inputs.
 ** 
 ** Supposes that all the inputs images already contains an image that looks as if taken from the `output_camera`. */
class blend_closest_filter : public mf::flow::filter {
public:
	using camera_type = mf::projection_image_camera<std::uint8_t>;

	struct input_visual {
		input_visual(blend_closest_filter& self, const camera_type& cam) :
			image_input(self), camera(cam) { }
		
		input_type<2, mf::masked_elem<mf::rgb_color>> image_input;
		parameter_type<camera_type> camera;
	};
	
	struct active_input_visual {
		active_input_visual(input_visual* vis, mf::real dist) :
			visual(vis), camera_distance(dist) { }
		
		input_visual* visual;
		mf::real camera_distance;
	};

private:
	std::size_t number_of_active_inputs_;
	std::vector<std::unique_ptr<input_visual>> visuals_;
	
	std::vector<active_input_visual> active_visuals_;

public:
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
	parameter_type<camera_type> output_camera;
		
	blend_closest_filter(mf::flow::filter_node& nd, std::size_t n = 3) :
		mf::flow::filter(nd), number_of_active_inputs_(n), output(*this) { }

	input_visual& add_input_visual(const camera_type& cam) {
		visuals_.emplace_back(new input_visual(*this, cam));
		return *visuals_.back();
	}

protected:
	void setup() override;
	void pre_process(mf::flow::node_job&) override;
	void process(mf::flow::node_job&) override;
};

#endif
