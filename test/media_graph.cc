#include <catch.hpp>
#include "../src/media_node.h"
#include "../src/yuv_file_source.h"
#include "../src/video_file_sink.h"
#include "../src/color_converter.h"

using namespace mf;

TEST_CASE("media_graph", "[media_graph]") {
	yuv_file_source source("/Users/timlenertz/Desktop/data/BBB_Flowers_cam0005.yuv", 1280, 768, 420);
	color_converter<ycbcr_color, rgb_color> converter(make_ndsize(768, 1280));
	video_file_sink sink("vid.mp4", make_ndsize(768, 1280));
	
	sink.image_input().connect(converter);
	converter.image_input().connect(source);
	
	for(int i = 0; i < 100; ++i) sink.pull();
}