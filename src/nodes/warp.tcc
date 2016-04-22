#include <algorithm>
#include "../ndarray/ndarray.h"

namespace mf { namespace node {

template<typename Color, typename Depth>
void warp<Color, Depth>::setup() {
	output.define_frame_shape(image_input.frame_shape());
}


template<typename Color, typename Depth>
void warp<Color, Depth>::pre_process() {
	output_camera->rotate_z_axis(0.1_deg);
	output_camera->move_z(-0.05);
}


template<typename Color, typename Depth>
void warp<Color, Depth>::process() {
	Eigen_projective3 homography = homography_transformation(*input_camera, *output_camera);
	std::cout << "homog=\n" << homography.matrix() << std::endl;
	
	std::fill(output.view().begin(), output.view().end(), background_color);

	ndarray<2, real> d_buffer(image_input.view().shape());
	for(real& d : d_buffer) d = 0.0;

	for(std::ptrdiff_t y = 0; y < image_input.view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < image_input.view().shape()[1]; ++x) {
		auto pix_c = make_ndptrdiff(y, x);
		
		Depth pix_d = depth_input.view().at(pix_c).intensity;
		Color col = image_input.view().at(pix_c);

		real d = input_camera->to_depth(pix_d);
		auto c = input_camera->to_image(pix_c);

		Eigen_vec3 in(c[0], c[1], d);
		Eigen_vec3 out = (homography * in.homogeneous()).eval().hnormalized();
		
		auto out_pix_c = output_camera->to_pixel(out.head(2));
		
		if(output_camera->image_span().includes(out_pix_c)) {
			real& old_d = d_buffer.at(out_pix_c);
			real new_d = out[2];
			if(new_d > old_d) {
				output.view().at(out_pix_c) = col;
				old_d = new_d;
			}
		}
	}
}


}}
