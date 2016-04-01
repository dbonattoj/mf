#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/color_converter.h"
#include "../src/nodes/depth_converter.h"
#include "../src/nodes/warp.h"
#include "../src/utility/misc.h"
#include "../src/config/camera_array.h"
#include <string>

using namespace mf;

std::string dir = "/data/test_sequences/bbb_flowers_noblur/";

std::string im1 = dir + "BBB_Flowers_cam0025.yuv";
std::string di1 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0025.yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam1_name = "param_ptm25";
std::string vcam_name = "param_ptm30";
std::string out = "output/video.avi";
auto shape = make_ndsize(768, 1280);
int sampling = 420;


int main() {
	camera_array cams(cams_file);
	projection_image_camera cam1(cams[cam1_name], false, shape[0], shape[1]);
	projection_image_camera vcam(cams[vcam_name], false, shape[0], shape[1]);
	
	media_graph graph;
	
	auto& im1_source = graph.add_node<node::importer<yuv_importer>>(im1, shape, sampling);
	auto& im1_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& di1_source = graph.add_node<node::importer<yuv_importer>>(di1, shape, sampling);
	auto& di1_converter = graph.add_node<node::color_converter<ycbcr_color, mono_color>>();
	auto& di1_dconverter = graph.add_node<node::depth_converter>();
	auto& warp = graph.add_node<node::warp<rgb_color>>();
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	
	im1_converter.input.connect(im1_source.output);
	di1_converter.input.connect(di1_source.output);
	di1_dconverter.input.connect(di1_converter.output);
	warp.image_input.connect(im1_converter.output);
	warp.depth_input.connect(di1_dconverter.output);
	sink.input.connect(warp.output);
	
	di1_dconverter.z_far = 0.0;
	di1_dconverter.z_near = 0.0;
	warp.input_camera = &cam1;
	warp.output_camera = &vcam;
	
	graph.setup();
	graph.run();
}
