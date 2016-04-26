#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/ndarray/ndarray_view.h"
#include "../src/ndarray/ndarray.h"
#include "../src/point_cloud/point.h"
#include "../src/point_cloud/point_cloud.h"
#include "../src/config/tanimoto_camera_array.h"
#include "../src/camera/projection_image_camera.h"
#include "../src/color.h"
#include "../src/flow/graph.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/color_converter.h"
#include "../src/nodes/blender.h"
#include "../src/nodes/warp.h"
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
std::string out = "output/video.avi";


std::string L = "2", R = "6", V = "4";

std::string im1 = dir + im+L+".yuv";
std::string di1 = dir + di+L+".yuv";
std::string im2 = dir + im+R+".yuv";
std::string di2 = dir + di+R+".yuv";
std::string cams_file = dir + cm;
std::string cam1_name = pr+L;
std::string cam2_name = pr+R;
std::string vcam_name = pr+V;
auto shape = make_ndsize(h, w);

int main() {
	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	tanimoto_camera_array cams(cams_file, dparam, {w, h});
	projection_image_camera<std::uint8_t> cam1(cams[cam1_name], make_ndsize(w, h));
	projection_image_camera<std::uint8_t> cam2(cams[cam2_name], make_ndsize(w, h));
	projection_image_camera<std::uint8_t> vcam(cams[vcam_name], make_ndsize(w, h));
	cam1.flip_pixel_coordinates();
	cam2.flip_pixel_coordinates();
	vcam.flip_pixel_coordinates();

	flow::graph graph;
	
	auto& im1_source = graph.add_node<node::importer<yuv_importer>>(im1, shape, sampling);
	auto& im1_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& di1_source = graph.add_node<node::importer<yuv_importer>>(di1, shape, sampling);
	auto& di1_converter = graph.add_node<node::color_converter<ycbcr_color, std::uint8_t>>();
	auto& warp1 = graph.add_node<node::warp<rgb_color, std::uint8_t>>(cam1, vcam);
	im1_converter.in.connect(im1_source.out);
	di1_converter.in.connect(di1_source.out);
	warp1.image_in.connect(im1_converter.out);
	warp1.depth_in.connect(di1_converter.out);
	

	auto& im2_source = graph.add_node<node::importer<yuv_importer>>(im2, shape, sampling);
	auto& im2_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& di2_source = graph.add_node<node::importer<yuv_importer>>(di2, shape, sampling);
	auto& di2_converter = graph.add_node<node::color_converter<ycbcr_color, std::uint8_t>>();
	auto& warp2 = graph.add_node<node::warp<rgb_color, std::uint8_t>>(cam2, vcam);
	im2_converter.in.connect(im2_source.out);
	di2_converter.in.connect(di2_source.out);
	warp2.image_in.connect(im2_converter.out);
	warp2.depth_in.connect(di2_converter.out);


	auto& blender = graph.add_node<node::blender<rgb_color>>();
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	blender.in1.connect(warp1.out);
	blender.in2.connect(warp2.out);
	sink.in.connect(blender.out);
/*
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	sink.input.connect(warp1.output);
*/
	
	graph.setup();
	graph.run();
}
