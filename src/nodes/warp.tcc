#include <algorithm>

namespace mf { namespace node {

template<typename Color>
void warp<Color>::setup_() {
	output.define_frame_shape(image_input.frame_shape());
}


template<typename Color>
void warp<Color>::process_() {
	auto im_in = flip(image_input.view());
	auto di_in = flip(depth_input.view());
	auto out = flip(output.view());
	
	Eigen::Projective3f homography = homography_transformation(*input_camera, *output_camera);

	std::fill(out.begin(), out.end(), background_color);
	
	ndptrdiff<2> in_pix;
	
	for(in_pix[0] = 0; in_pix[0] != im_in.shape()[0]; ++in_pix[0])
	for(in_pix[1] = 0; in_pix[1] != im_in.shape()[1]; ++in_pix[1]) {	
		Eigen_scalar in_d = di_in.at(in_pix);
		const Color& in_col = im_in.at(in_pix);
		
		auto in_proj2 = input_camera->to_projected(in_pix);
		auto in_proj3 = Eigen_vec4(in_proj2[0], in_proj2[1], 1.0, 1.0/in_d);
				
		Eigen_vec3 out_proj3 = (homography * in_proj3).eval().hnormalized();
		
		auto out_pix = output_camera->to_image(Eigen_vec2(out_proj3.head<2>()));
		if(output_camera->in_bounds(out_pix))
			out.at(out_pix) = in_col;
	}
}


}}
