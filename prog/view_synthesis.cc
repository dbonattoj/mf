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

#include <mf/io/yuv_importer.h>
#include <mf/io/video_exporter.h>
#include <mf/color.h>
#include <mf/flow/graph.h>
#include <mf/flow/sync_node.h>
#include <mf/flow/async_node.h>
#include <mf/filter/importer.h>
#include <mf/filter/exporter.h>
#include <mf/filter/color_converter.h>

#include "support/input_data.h"

#include "filter/blend_closest.h"
#include "filter/blend_depth_maps.h"
#include "filter/inpaint.h"
#include "filter/result_improve.h"
#include "filter/depth_map_improve.h"
#include "filter/homography_depth_warp.h"
#include "filter/reverse_homography_warp.h"

#include <string>

using namespace mf;

std::string out = "view_synthesis_video.avi";

input_data data = poznan_blocks();

int main(int argc, const char* argv[]) {
	sleep(10);
	
	if(argc >= 2) out = argv[1];
		
	flow::graph graph;
	
	// camera as function of time
	// varies between poses of left-most and right-most cameras
	// sinusodial time function, slerp to interpolate orientation
	auto camera_at_time = [&](time_unit t) -> camera_type {
		const camera_type& leftmost_cam = data.visuals[2].camera;
		const camera_type& rightmost_cam = data.visuals[data.visuals.size() - 2].camera;
		const camera_type& middle_cam = data.visuals[data.visuals.size() / 2].camera;
		
		Eigen_vec3 min_pos = leftmost_cam.absolute_pose().position;
		Eigen_quaternion min_ori = leftmost_cam.absolute_pose().orientation;

		Eigen_vec3 max_pos = rightmost_cam.absolute_pose().position;
		Eigen_quaternion max_ori = rightmost_cam.absolute_pose().orientation;
	
		real k = (1.0 + std::sin(pi * t / 100.0)*0.7) / 2.0;
	
		Eigen_vec3 pos = min_pos + k*(max_pos - min_pos);
		Eigen_quaternion ori = min_ori.slerp(k, max_ori);
	
		camera_type output_cam = middle_cam;
		pose ps(pos, ori);
		output_cam.set_relative_pose(ps);
		return output_cam;
	};

	auto shape = make_ndsize(data.image_height, data.image_width);


	auto& blender = graph.add_filter<blend_closest_filter>(3);
	blender.output_camera.set_time_function(camera_at_time);

	
	for(const input_visual& vis : data.visuals) {
		// Source+converter for image
		auto& im_source = graph.add_filter<flow::importer_filter<yuv_importer>>(
			vis.image_yuv_file, shape, data.yuv_sampling
		);
		auto& im_converter = graph.add_filter<flow::color_converter_filter<ycbcr_color, rgb_color>>();
		im_converter.input.connect(im_source.output);
		
		// Source+converter for depth image
		auto& di_source = graph.add_filter<flow::importer_filter<yuv_importer>>(
			vis.depth_image_yuv_file, shape, data.yuv_sampling
		);
		auto& di_converter = graph.add_filter<flow::color_converter_filter<ycbcr_color, masked_elem<depth_type>>>();
		di_converter.input.connect(di_source.output);
		
		// Forward homography warp of depth map alone
		auto& di_warper = graph.add_filter<homography_depth_warp_filter>();
		di_warper.source_depth_input.connect(di_converter.output);
		di_warper.source_camera.set_constant(vis.camera);
		di_warper.destination_camera.set_mirror(blender.output_camera);

		// Filter on the depth map
		auto& di_filter = graph.add_filter<depth_map_improve_filter, flow::async_node>(3);
		di_filter.input.connect(di_warper.destination_depth_output);
		
		// Reverse homography warp
		auto& warper = graph.add_filter<reverse_homography_warp_filter, flow::async_node>();
		warper.source_image_input.connect(im_converter.output);
		warper.destination_depth_input.connect(di_filter.output);		
		warper.source_camera.set_constant(vis.camera);
		warper.destination_camera.set_mirror(blender.output_camera);

		// Connect to one blender visual
		auto& blender_vis = blender.add_input_visual(vis.camera);
		blender_vis.image_input.connect(warper.destination_image_output);
	}

	// Result filter
	auto& result_filter = graph.add_filter<result_improve_filter>();
	result_filter.input.connect(blender.output);

	// Converter+sink for output image
	auto& sink = graph.add_sink_filter<flow::exporter_filter<video_exporter>>(out, shape);
	sink.input.connect(result_filter.output);

	// Setup the graph
	graph.setup();
	
	// Run the graph
	graph.callback_function = [](time_unit t) {
		std::cout << "frame " << t << "..." << std::endl;
	};
	graph.run();
}