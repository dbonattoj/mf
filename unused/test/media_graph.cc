/*
#include <catch.hpp>
#include "../src/media_node.h"
#include "../src/yuv_file_source.h"
#include "../src/video_file_sink.h"
#include "../src/color_converter.h"

using namespace mf;

class effect : public media_node_sequential {	
public:
	media_node_output<2, rgb_color> output;
	media_node_input<2, rgb_color> input;

	effect(const ndsize<2>& shape) :
	output(*this, shape), input(6, 6) {
		register_input_(input);
		register_output_(output);
	}
	
	void process_() override {	
		auto shp = input.view().shape();
		for(std::ptrdiff_t y = 0; y < shp[1]; ++y)
		for(std::ptrdiff_t x = 0; x < shp[2]; ++x) {
			std::vector<uint8_t> rs, gs, bs;
			for(std::ptrdiff_t t = 0; t < shp[0]; ++t) {
				rgb_color p = input.view()[t][y][x];
				rs.push_back(p.r);
				gs.push_back(p.g);
				bs.push_back(p.b);
			}
			std::size_t n = rs.size();
			std::nth_element(rs.begin(), rs.begin() + n/2, rs.end());
			std::nth_element(gs.begin(), gs.begin() + n/2, gs.end());
			std::nth_element(bs.begin(), bs.begin() + n/2, bs.end());
			rgb_color mix;
			mix.r = rs[n/2];
			mix.g = gs[n/2];
			mix.b = bs[n/2];
			output.view()[y][x] = mix;
		}
	}
};


TEST_CASE("media_graph", "[media_graph]") {
	yuv_file_source source("/Users/timlenertz/Desktop/data/BBB_Flowers_cam0084.yuv", 1280, 768, 420);
	color_converter<ycbcr_color, rgb_color> converter(make_ndsize(768, 1280));
	effect effect(make_ndsize(768, 1280));
	video_file_sink sink("vid.mp4", make_ndsize(768, 1280));
	
	sink.input.connect(effect.output);
	effect.input.connect(converter.output);
	converter.input.connect(source.output);
	
	for(int i = 0; i < 100; ++i) sink.pull_frame();
}
*/
