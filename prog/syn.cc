#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/blender.h"
#include "../src/nodes/color_converter.h"
#include "../src/nodes/depth_converter.h"
#include "../src/nodes/warp.h"
#include "../src/utility/misc.h"
#include "../src/config/camera_array.h"
#include <string>

using namespace mf;

//std::string dir = "/data/test_sequences/bbb_flowers_noblur/";
std::string dir = "/Users/timlenertz/Desktop/mf/data/";

std::string L = "05", R = "05", V = "05";

std::string im1 = dir + "BBB_Flowers_cam00"+L+".yuv";
std::string di1 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam00"+L+".yuv";
std::string im2 = dir + "BBB_Flowers_cam00"+R+".yuv";
std::string di2 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam00"+R+".yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam1_name = "param_ptm"+L;
std::string cam2_name = "param_ptm"+R;
std::string vcam_name = "param_ptm"+V;
std::string out = "output/video.avi";
std::size_t w = 1280, h = 768;
auto shape = make_ndsize(h, w);
int sampling = 420;

float z_far = 700.0, z_near = 0.2;

int main() {
	camera_array cams(cams_file);
	projection_image_camera cam1(cams[cam1_name], false, w, h);
	projection_image_camera cam2(cams[cam2_name], false, w, h);
	projection_image_camera vcam(cams[vcam_name], false, w, h);
	
	media_graph graph;
	
	auto& im1_source = graph.add_node<node::importer<yuv_importer>>(im1, shape, sampling);
	auto& im1_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& di1_source = graph.add_node<node::importer<yuv_importer>>(di1, shape, sampling);
	auto& di1_converter = graph.add_node<node::color_converter<ycbcr_color, mono_color>>();
	auto& di1_dconverter = graph.add_node<node::depth_converter>();
	auto& warp1 = graph.add_node<node::warp<rgb_color>>();
	im1_converter.input.connect(im1_source.output);
	di1_converter.input.connect(di1_source.output);
	di1_dconverter.input.connect(di1_converter.output);
	warp1.image_input.connect(im1_converter.output);
	warp1.depth_input.connect(di1_dconverter.output);
	di1_dconverter.z_far = z_far;
	di1_dconverter.z_near = z_near;
	warp1.input_camera = &cam1;
	warp1.output_camera = &vcam;
/*
	auto& im2_source = graph.add_node<node::importer<yuv_importer>>(im2, shape, sampling);
	auto& im2_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& di2_source = graph.add_node<node::importer<yuv_importer>>(di2, shape, sampling);
	auto& di2_converter = graph.add_node<node::color_converter<ycbcr_color, mono_color>>();
	auto& di2_dconverter = graph.add_node<node::depth_converter>();
	auto& warp2 = graph.add_node<node::warp<rgb_color>>();
	im2_converter.input.connect(im2_source.output);
	di2_converter.input.connect(di2_source.output);
	di2_dconverter.input.connect(di2_converter.output);
	warp2.image_input.connect(im2_converter.output);
	warp2.depth_input.connect(di2_dconverter.output);
	di2_dconverter.z_far = z_far;
	di2_dconverter.z_near = z_near;
	warp2.input_camera = &cam2;
	warp2.output_camera = &vcam;


	auto& blender = graph.add_node<node::blender<rgb_color>>();
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	blender.input1.connect(warp1.output);
	blender.input2.connect(warp2.output);
	sink.input.connect(blender.output);
*/
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	sink.input.connect(warp1.output);
	
	graph.setup();
	graph.run();
}
