#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter_node.h"
#include "../src/nodes/video_file_sink.h"
#include "../src/utility/misc.h"

using namespace mf;

char im[] = "/data/test_sequences/bbb_flowers_noblur/BBB_Flowers_cam0020.yuv";
int w = 1280;
int h = 768;


int main() {
	media_graph graph;
	auto& im_source = graph.add_node<yuv_file_source>(im, w, h, 420);
	auto& im_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& sink = graph.add_sink<video_file_sink>("output/video.avi");
	im_converter.input.connect(im_source.output);
	sink.input.connect(im_converter.output);
	graph.setup();
	graph.run();
}
