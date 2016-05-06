#include <mf/io/yuv_importer.h>
#include <mf/io/video_exporter.h>
#include <mf/ndarray/ndarray_view.h>
#include <mf/ndarray/ndarray.h>
#include <mf/point_cloud/point.h>
#include <mf/point_cloud/point_cloud.h>
#include <mf/config/tanimoto_camera_array.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>
#include <mf/flow/graph.h>
#include <mf/nodes/importer.h>
#include <mf/nodes/exporter.h>
#include <mf/nodes/color_converter.h>
#include <mf/nodes/warp.h>
#include "support/blend_closest_node.h"
#include <string>

using namespace mf;

std::string dir = "/data/test_sequences/poznan_blocks/";
//std::string dir = "/Users/timlenertz/Desktop/mf/data/";
std::string im = "Poznan_Blocks_1920x1080_texture_cam";
std::string di = "Poznan_Blocks_1920x1080_depth_cf420_8bps_cam";
std::string cm = "Poznan_Blocks_Physical_and_Virtual_View_camera_parameters.txt";
std::string pr = "param_cam";
std::size_t w = 1920, h = 1080;
int sampling = 420;
float z_far = 100.0, z_near = 15.0;
std::string out = "output/video4.avi";

using camera_type = projection_image_camera<std::uint8_t>;

auto shape = make_ndsize(h, w);

camera_type camera_at_time(time_unit t) {
	Eigen_vec3 min_pos = visuals_.front()->camera.absolute_pose().position;
	Eigen_quaternion min_ori = visuals_.front()->camera.absolute_pose().orientation;

	Eigen_vec3 max_pos = visuals_.back()->camera.absolute_pose().position;
	Eigen_quaternion max_ori = visuals_.back()->camera.absolute_pose().orientation;
	
	real k = (1.0 + std::cos(pi * t / 200.0)*0.7) / 2.0;
	
	Eigen_vec3 pos = min_pos + k*(max_pos - min_pos);
	Eigen_quaternion ori = min_ori.slerp(k, max_ori);
	
	pose ps(pos, ori);
	output_camera_.set_relative_pose(ps);
}



int main() {
	flow::graph graph;

	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	tanimoto_camera_array cams(dir + cm, dparam, {w, h});
	projection_image_camera<std::uint8_t> vcam(cams[pr + "5"], make_ndsize(w, h));
	vcam.flip_pixel_coordinates();
	
	auto& blender = graph.add_node<node::blend_closest_node>(vcam);
	
	for(int i = 2; i <= 8; ++i) {
		std::string cam_name = pr + std::to_string(i);
		projection_image_camera<std::uint8_t> cam(cams[cam_name], make_ndsize(w, h));
		cam.flip_pixel_coordinates();
		
		std::string im_file = dir + im + std::to_string(i) + ".yuv";
		std::string di_file = dir + di + std::to_string(i) + ".yuv";
		
		auto& im_source = graph.add_node<node::seekable_importer<yuv_importer>>(im_file, shape, sampling);
		auto& im_converter = graph.add_node<node::color_converter<ycbcr_color, rgba_color>>();
		auto& di_source = graph.add_node<node::seekable_importer<yuv_importer>>(di_file, shape, sampling);
		auto& di_converter = graph.add_node<node::color_converter<ycbcr_color, std::uint8_t>>();
		auto& warper = graph.add_node<node::warp<rgba_color, std::uint8_t>>(cam, vcam);

		im_converter.input.connect(im_source.output);
		di_converter.input.connect(di_source.output);
		warper.image_input.connect(im_converter.output);
		warper.depth_input.connect(di_converter.output);
		
		auto& vis = blender.add_input_visual(cam);
		vis.image_input.connect(warper.output);
	}

	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	auto& sink_converter = graph.add_node<node::color_converter<rgba_color, rgb_color>>();
	sink_converter.input.connect(blender.output);
	sink.input.connect(sink_converter.output);
	
	graph.setup();
	graph.run();
}
