#include <iostream>

namespace mf {

template<typename Color>
void warp_node<Color>::setup_() {
	output.define_frame_shape(image_input.frame_shape());
}


template<typename Color>
void warp_node<Color>::process_() {
	Eigen::Projective3f homography = homography_transformation(*input_camera, *output_camera);
	std::cout << homography.matrix() << std::endl;

	for(std::ptrdiff_t y = 0; y != image_input.view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x != image_input.view().shape()[1]; ++x)
		output.view()[y][x] = { 0, 0, 0 };

	
	for(std::ptrdiff_t y = 0; y != image_input.view().shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x != image_input.view().shape()[1]; ++x) {		
		auto in_p2 = input_camera->to_projected({x, y});
		float in_d = depth_input.view()[y][x].intensity;
		Eigen::Vector3f in_p3(in_p2[0], in_p2[1], in_d);
		
		Eigen::Vector3f out_p3 = (homography * in_p3.homogeneous()).eval().hnormalized();
		
		auto out_ip = output_camera->to_image(Eigen::Vector2f{ out_p3[0], out_p3[1] });
		if(output_camera->in_bounds(out_ip)) {
			std::ptrdiff_t ox = out_ip[0], oy = out_ip[1];
			output.view()[oy][ox] = image_input.view()[oy][ox];
		}
	}
}


}