#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter_node.h"
#include "../src/nodes/video_file_sink.h"
#include "../src/util.h"

using namespace mf;

char im[] = "/data/test_sequences/bbb_flowers_noblur/BBB_Flowers_cam0010.yuv";
char di[] = "/data/test_sequences/bbb_flowers_noblur/BBB_Flowers_1280x768_Depth_8bps_cf420_cam0010.yuv";
int w = 1280;
int h = 768;


class d_effect_node : public media_node {	
public:
	media_node_output<2, rgb_color> output;
	media_node_input<2, rgb_color> im_input;
	media_node_input<2, mono_color> di_input;

	d_effect_node() :
		media_node(0), output(*this), im_input(*this), di_input(*this) { }
	
	void setup_() override {
		output.define_frame_shape(im_input.frame_shape());
	}
	
	void process_() override {
		std::transform(
			im_input.view().begin(),
			im_input.view().end(),
			di_input.view().begin(),
			output.view().begin(),
			[](rgb_color col, mono_color d) -> rgb_color {
				float f = clamp<float>(d.intensity / 100.0f, 0.4f, 1.0f);
				col.r = clamp<std::uint8_t>(f*col.r, 0, 255);
				col.g = clamp<std::uint8_t>(f*col.g, 0, 255);
				col.b = clamp<std::uint8_t>(f*col.b, 0, 255);
				return col;
			}
		);
	}
};


class b_effect_node : public media_node {	
public:
	media_node_output<2, rgb_color> output;
	media_node_input<2, rgb_color> input;

	b_effect_node() :
		media_node(2), output(*this), input(*this, 6, 6) { }

	void setup_() override {
		output.define_frame_shape(input.frame_shape());
	}	

	void process_() override {	
		auto shp = input.full_view().shape();
		for(std::ptrdiff_t y = 0; y < shp[1]; ++y)
		for(std::ptrdiff_t x = 0; x < shp[2]; ++x) {
			float tr = 0, tg = 0, tb = 0;
			float c = shp[0];
			for(std::ptrdiff_t t = 0; t < shp[0]; ++t) {
				rgb_color p = input.full_view()[t][y][x];
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
	auto& im_source = graph.add_node<yuv_file_source>(im, w, h, 420);
	auto& di_source = graph.add_node<yuv_file_source>(di, w, h, 420);
	auto& im_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& di_converter = graph.add_node<color_converter_node<ycbcr_color, mono_color>>();
	auto& b_effect = graph.add_node<b_effect_node>();
	auto& d_effect = graph.add_node<d_effect_node>();
	auto& sink = graph.add_sink<video_file_sink>("output/video.avi");
	im_converter.input.connect(im_source.output);
	di_converter.input.connect(di_source.output);
	b_effect.input.connect(im_converter.output);
	d_effect.im_input.connect(b_effect.output);
	d_effect.di_input.connect(di_converter.output);
	sink.input.connect(d_effect.output);
	graph.setup();
	graph.run();
}
