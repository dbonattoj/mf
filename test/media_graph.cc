#include <catch.hpp>
#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/graph/media_sequential_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter.h"
#include "../src/nodes/video_file_sink.h"

using namespace mf;

char input[] = "/Users/timlenertz/Desktop/data/BBB_Flowers_cam0084.yuv";

TEST_CASE("media graph", "[media_graph]") {
	media_graph graph;
	auto& source = graph.add_node<yuv_file_source>(input, 1280, 768, 420);
	auto& converter = graph.add_node<color_converter<ycbcr_color, rgb_color>>();
	auto& sink = graph.add_sink<video_file_sink>("video.mp4");
	converter.input.connect(source.output);
	sink.input.connect(converter.output);
	graph.setup();
	graph.run();
}
