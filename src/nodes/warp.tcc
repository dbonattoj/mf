#include <algorithm>
#include "../ndarray/ndarray.h"

namespace mf { namespace node {

template<typename Color, typename Depth>
void warp<Color, Depth>::setup() {
	out.define_frame_shape(image_in.frame_shape());
}


template<typename Color, typename Depth>
void warp<Color, Depth>::process() {
	Eigen_projective3 homography = homography_transformation(input_camera_, output_camera_);
	
	std::fill(out.view().begin(), out.view().end(), Color::null());

	ndarray<2, real> d_buffer(image_in.view().shape());
	for(real& d : d_buffer) d = 0.0;

	for(std::ptrdiff_t y = 0; y < image_in.view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < image_in.view().shape()[1]; ++x) {
		auto pix_c = make_ndptrdiff(y, x);
		
		Depth pix_d = depth_in.view().at(pix_c);
		Color col = image_in.view().at(pix_c);

		real d = input_camera_.to_depth(pix_d);
		auto c = input_camera_.to_image(pix_c);

		Eigen_vec3 in_p(c[0], c[1], d);
		Eigen_vec3 out_p = (homography * in_p.homogeneous()).eval().hnormalized();
		
		auto out_pix_c = output_camera_.to_pixel(out_p.head(2));
		
		if(output_camera_.image_span().includes(out_pix_c)) {
			real& old_d = d_buffer.at(out_pix_c);
			real new_d = out_p[2];
			if(new_d > old_d) {
				out.view().at(out_pix_c) = col;
				old_d = new_d;
			}
		}
	}
}


}}
