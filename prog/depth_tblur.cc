#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/graph/media_sequential_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter_node.h"
#include "../src/nodes/video_file_sink.h"
#include "../src/util.h"

using namespace mf;

char im[] = "data/BBB_Flowers_cam0005.yuv";
char di[] = "data/BBB_Flowers_1280x768_Depth_8bps_cf420_cam0005.yuv";


class b_effect_node : public media_sequential_node {	
public:
	media_node_output<2, rgb_color> output;
	media_node_input<2, rgb_color> im_input;
	media_node_input<2, mono_color> di_input;

	b_effect_node() :
		output(*this), im_input(*this, 12, 12), di_input(*this) { }

	void setup_() override {
		output.define_frame_shape(im_input.frame_shape());
	}	

	void process_() override {	
		auto shp = im_input.full_view().shape();
		for(std::ptrdiff_t y = 0; y < shp[1]; ++y)
		for(std::ptrdiff_t x = 0; x < shp[2]; ++x) {
			float depth = di_input.view()[y][x].intensity;
			int mblur = 24 * (depth / 255.0f);
			mblur=1;
			int mn = clamp<int>(im_input.full_view_center() - mblur/2, 0, shp[0]-1);
			int mx = clamp<int>(im_input.full_view_center() + mblur/2, 0, shp[0]-1);
			float tr = 0, tg = 0, tb = 0;
			float c = mx - mn + 1;
			for(std::ptrdiff_t t = mn; t <= mx; ++t) {
				rgb_color p = im_input.full_view()[t][y][x];
				tr += p.r; tg += p.g; tb += p.b;
			}
			rgb_color mix;
			mix.r = tr / c; mix.g = tg / c; mix.b = tb / c;
			output.view()[y][x] = mix;
		}
	}
};


int main() {
	media_graph graph;
	auto& im_source = graph.add_node<yuv_file_source>(im, 1280, 768, 420);
	auto& di_source = graph.add_node<yuv_file_source>(di, 1280, 768, 420);
	auto& im_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& di_converter = graph.add_node<color_converter_node<ycbcr_color, mono_color>>();
	auto& b_effect = graph.add_node<b_effect_node>();
	auto& sink = graph.add_sink<video_file_sink>("video.mp4");
	im_converter.input.connect(im_source.output);
	di_converter.input.connect(di_source.output);
	b_effect.im_input.connect(im_converter.output);
	b_effect.di_input.connect(di_converter.output);
	sink.input.connect(b_effect.output);
	graph.setup();
	graph.run();
}
