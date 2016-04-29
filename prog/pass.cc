#include "../src/flow/graph.h"
#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/color_converter.h"
#include "../src/utility/misc.h"
#include <string>

using namespace mf;

/*
std::string dir = "/Users/timlenertz/Desktop/mf/data/";
std::string im1 = dir+"BBB_Flowers_cam0005.yuv";
std::size_t w = 1280, h = 768;
int sampling = 420;
std::string out = "output/_video.avi";
auto shape = make_ndsize(h, w);
*/


std::string dir = "/data/test_sequences/poznan_blocks/";
std::string im = "Poznan_Blocks_1920x1080_texture_cam";
std::string di = "Poznan_Blocks_1920x1080_depth_cf420_8bps_cam";
std::string cm = "Poznan_Blocks_Physical_and_Virtual_View_camera_parameters.txt";
std::string pr = "param_cam";
std::size_t w = 1920, h = 1080;
int sampling = 420;
float z_far = 100.0, z_near = 15.0;
std::string out = "output/video.avi";
std::string L = "2";

std::string im1 = dir + im+L+".yuv";
std::string di1 = dir + di+L+".yuv";
auto shape = make_ndsize(h, w);


int main() {
	flow::graph graph;
	
	auto& im1_source = graph.add_node<node::importer<yuv_importer>>(im1, shape, sampling);
	auto& im1_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	
	im1_converter.in.connect(im1_source.out);
	sink.in.connect(im1_converter.out);
		
	graph.setup();
	graph.run();
}
