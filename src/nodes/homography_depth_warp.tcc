#include <algorithm>
#include "../ndarray/ndarray.h"

namespace mf { namespace node {

template<typename Depth>
void homography_depth_warp<Depth>::setup() {
	destination_depth_output.define_frame_shape(source_depth_input.frame_shape());
}


template<typename Depth>
void homography_depth_warp<Depth>::process(flow::node_job& job) {	
	auto out = job.out(destination_depth_output);
	auto depth_in = job.in(source_depth_input);
	auto in_cam = job.param(source_camera);
	auto out_cam = job.param(destination_camera);
	
	auto image_shape = depth_in.shape();
		
	Eigen_projective3 homography = homography_transformation(in_cam, out_cam);
	
	std::fill(out.begin(), out.end(), masked_depth_type::null());

	for(std::ptrdiff_t y = 0; y < image_shape[0]; ++y)
	for(std::ptrdiff_t x = 0; x < image_shape[1]; ++x) {
		auto source_pix_coord = make_ndptrdiff(y, x);
		masked_depth_type source_pix_depth = depth_in.at(source_pix_coord);
		if(source_pix_depth.is_null()) continue;

		real source_depth = in_cam.to_depth(source_pix_depth);
		auto source_coord = in_cam.to_image(source_pix_coord);

		Eigen_vec3 source_3coord(source_coord[0], source_coord[1], source_depth);
		Eigen_vec3 dest_3coord = (homography * source_3coord.homogeneous()).eval().hnormalized();

		real dest_depth = dest_3coord[2];
		
		auto dest_pix_coord = out_cam.to_pixel(dest_3coord.head(2));
		Depth dest_pix_depth = out_cam.to_pixel_depth(dest_depth);
		
		if(out_cam.image_span().includes(dest_pix_coord)) {
			masked_depth_type& output_dest_pix_depth = out.at(dest_pix_coord);
			if(output_dest_pix_depth.is_null() || dest_pix_depth > output_dest_pix_depth.elem)
				output_dest_pix_depth = dest_pix_depth;
		}
	}
}


}}
