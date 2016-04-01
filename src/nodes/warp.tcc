#include <algorithm>

namespace mf { namespace node {

template<typename Color>
void warp<Color>::setup_() {
	output.define_frame_shape(image_input.frame_shape());
}


template<typename Color>
void warp<Color>::process_() {
	Eigen::Projective3f homography = homography_transformation(*input_camera, *output_camera);
	MF_DEBUG(homography.matrix());

	std::fill(output.view().begin(), output.view().end(), background_color);
	
	ndptrdiff<2> in_pix;
	
	for(in_pix[0] = 0; in_pix[0] != image_input.view().shape()[0]; ++in_pix[0])
	for(in_pix[1] = 0; in_pix[1] != image_input.view().shape()[1]; ++in_pix[1]) {	
		Eigen_scalar in_d = depth_input.view().at(in_pix);
		const Color& in_col = image_input.view().at(in_pix);
		
		auto in_proj2 = input_camera->to_projected(flip(in_pix));
		auto in_proj3 = Eigen_vec3(in_proj2[0], in_proj2[1], in_d);
				
		Eigen_vec3 out_proj3 = (homography * in_proj3.homogeneous()).eval().hnormalized();
		
		auto out_pix = output_camera->to_image(Eigen_vec2(out_proj3.head<2>()));
		if(output_camera->in_bounds(out_pix))
			output.view().at(flip(out_pix)) = in_col;
	}
}


}}
