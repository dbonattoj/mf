#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/color_converter.h"
#include "../src/utility/misc.h"
#include <string>

using namespace mf;

std::string dir = "/data/test_sequences/bbb_flowers_noblur/";

std::string im1 = dir + "BBB_Flowers_cam0025.yuv";
std::string out = "output/video.avi";
auto shape = make_ndsize(768, 1280);
int sampling = 420;


int main() {
	media_graph graph;
	
	auto& im1_source = graph.add_node<node::importer<yuv_importer>>(im1, shape, sampling);
	auto& im1_converter = graph.add_node<node::color_converter<ycbcr_color, rgb_color>>();
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	
	im1_converter.input.connect(im1_source.output);
	sink.input.connect(im1_converter.output);
		
	graph.setup();
	graph.run();
}
