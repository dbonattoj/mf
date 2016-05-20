/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
