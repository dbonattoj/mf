namespace mf {

template<typename Pixel, typename Image_camera>
ndsize<2> backproject_node<Pixel, Image_camera>::shape_() const {
	return point_cloud_output.frame_shape();
}


template<typename Pixel, typename Image_camera>
void backproject_node<Pixel, Image_camera>::setup_() {
	point_cloud_output.define_frame_shape(image_input.frame_shape());
}


template<typename Pixel, typename Image_camera>
void backproject_node<Pixel, Image_camera>::process_() {
	using pixel_coordinates_type = typename Image_camera::pixel_coordinates_type;
	
	for(std::ptrdiff_t y = 0; y < shape_()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < shape_()[1]; ++x) {
		ndptrdiff<2> pix_c(x, y);
		Eigen_vec2 proj_c = camera_.to_projected(pixel_coord);

		float d = depth_input.view().at(pix_c);
		Pixel pix = image_input.view().at(pix_c);
				
		Eigen_vec3 world_c = camera_.point_with_projected_depth(proj_c, d);
		
		point_cloud_output.view().at(pix_c) = make_elem_tuple(world_c, pix);
	}
}

}