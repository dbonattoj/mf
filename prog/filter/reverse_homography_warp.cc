#include "reverse_homography_warp.h"
#include <algorithm>
#include <mf/ndarray/ndarray.h>
#include <mf/ndarray/ndarray_filter.h>
#include <mf/image/kernel.h>

using namespace mf;

void reverse_homography_warp_filter::setup() {
	destination_image_output.define_frame_shape(source_image_input.frame_shape());
}


void reverse_homography_warp_filter::process(mf::flow::node_job& job) {	
	auto out = job.out(destination_image_output);
	auto depth_in = job.in(destination_depth_input);
	auto image_in = job.in(source_image_input);
	auto in_cam = job.param(source_camera);
	auto out_cam = job.param(destination_camera);
	
	auto image_shape = image_in.shape();
		
	Eigen_projective3 reverse_homography = homography_transformation(out_cam, in_cam);

	auto kernel = box_image_kernel(3);
	
	for(std::ptrdiff_t y = 0; y < image_shape[0]; ++y)
	for(std::ptrdiff_t x = 0; x < image_shape[1]; ++x) {
		auto dest_pix_coord = make_ndptrdiff(y, x);
		masked_depth_type dest_pix_depth = depth_in.at(dest_pix_coord);
		if(dest_pix_depth.is_null()) {
			out.at(dest_pix_coord) = masked_image_type::null();
			continue;
		}
		
		real dest_depth = out_cam.to_depth(dest_pix_depth);
		auto dest_coord = out_cam.to_image(dest_pix_coord);

		Eigen_vec3 dest_3coord(dest_coord[0], dest_coord[1], dest_depth);
		Eigen_vec3 source_3coord = (reverse_homography * dest_3coord.homogeneous()).eval().hnormalized();

		auto source_pix_coord = in_cam.to_pixel(source_3coord.head(2));
		
		if(in_cam.image_span().includes(source_pix_coord)) {
			color_type source_color = image_in.at(source_pix_coord);
			
			auto p = place_kernel_at(out, kernel.cview(), dest_pix_coord);
			std::fill(p.view_section.begin(), p.view_section.end(), source_color);
		} else {
			out.at(dest_pix_coord) = masked_image_type::null();
		}
	}
}
