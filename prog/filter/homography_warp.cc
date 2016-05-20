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

#include "homography_warp.h"
#include <algorithm>
#include <mf/ndarray/ndarray.h>

using namespace mf;


void homography_warp_filter::setup() {
	destination_image_output.define_frame_shape(source_image_input.frame_shape());
}



void homography_warp_filter::process(flow::node_job& job) {	
	auto out = job.out(destination_image_output);
	auto depth_in = job.in(source_depth_input);
	auto image_in = job.in(source_image_input);
	auto in_cam = job.param(source_camera);
	auto out_cam = job.param(destination_camera);
	
	auto image_shape = image_in.shape();
		
	Eigen_projective3 homography = homography_transformation(in_cam, out_cam);
	
	std::fill(out.begin(), out.end(), color_type::null());

	ndarray<2, real> d_buffer(image_in.shape());
	for(real& d : d_buffer) d = 0.0;

	for(std::ptrdiff_t y = 0; y < image_shape[0]; ++y)
	for(std::ptrdiff_t x = 0; x < image_shape[1]; ++x) {
		auto source_pix_coord = make_ndptrdiff(y, x);
		depth_type source_pix_depth = depth_in.at(source_pix_coord);
		color_type source_color = image_in.at(source_pix_coord);

		real source_depth = in_cam.to_depth(source_pix_depth);
		auto source_coord = in_cam.to_image(source_pix_coord);

		Eigen_vec3 source_3coord(source_coord[0], source_coord[1], source_depth);
		Eigen_vec3 dest_3coord = (homography * source_3coord.homogeneous()).eval().hnormalized();

		real dest_depth = dest_3coord[2];
		
		auto dest_pix_coord = out_cam.to_pixel(dest_3coord.head(2));
		
		if(out_cam.image_span().includes(dest_pix_coord)) {
			real& previous_dest_depth = d_buffer.at(dest_pix_coord);
			if(dest_depth > previous_dest_depth) {
				out.at(dest_pix_coord) = source_color;
				previous_dest_depth = dest_depth;
			}
		}
	}
}
