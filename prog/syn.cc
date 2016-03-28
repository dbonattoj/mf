#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter_node.h"
#include "../src/nodes/video_file_sink.h"
#include "../src/utility/misc.h"
#include "../src/config/camera_array.h"
#include "../src/nodes/warp_node.h"
#include <string>

using namespace mf;

char im[] = "/Users/timlenertz/Desktop/mf/data/BBB_Flowers_cam0005.yuv";


std::string dir = "/Users/timlenertz/Desktop/mf/data/";

std::string im1 = dir + "BBB_Flowers_cam0005.yuv";
std::string di1 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0005.yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam1_name = "param_ptm05";
std::string vcam_name = "param_ptm20";
int w = 1280;
int h = 768;
int sampling = 420;


int main() {
	camera_array cams(cams_file);
	projection_image_camera cam1(cams[cam1_name], false, w, h);
	projection_image_camera vcam(cams[vcam_name], false, w, h);
	
	media_graph graph;
	auto& im1_source = graph.add_node<yuv_file_source>(im1, w, h, sampling);
	auto& im1_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& di1_source = graph.add_node<yuv_file_source>(di1, w, h, sampling);
	auto& di1_converter = graph.add_node<color_converter_node<ycbcr_color, mono_color>>();
	auto& warp = graph.add_node<warp_node<rgb_color>>();
	auto& sink = graph.add_sink<video_file_sink>("output/video.avi");
	im1_converter.input.connect(im1_source.output);
	di1_converter.input.connect(di1_source.output);
	warp.image_input.connect(im1_converter.output);
	warp.depth_input.connect(di1_converter.output);
	warp.input_camera = &cam1;
	warp.output_camera = &vcam;
	sink.input.connect(warp.output);
	graph.setup();
	graph.run();
}
