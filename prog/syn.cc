#include "../src/graph/media_graph.h"
#include "../src/graph/media_node.h"
#include "../src/nodes/yuv_file_source.h"
#include "../src/nodes/color_converter_node.h"
#include "../src/nodes/video_file_sink.h"
#include "../src/utility/misc.h"
#include "../src/config/camera_array.h"
#include <string>

using namespace mf;

std::string dir = "/data/test_sequences/bbb_flowers_noblur/";

std::string im1 = dir + "BBB_Flowers_cam0020.yuv";
std::string im2 = dir + "BBB_Flowers_cam0030.yuv";
std::string di1 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0020.yuv";
std::string di2 = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0030.yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam1_name = "param_ptm20";
std::string cam2_name = "param_ptm30";
std::string vcam_name = "param_ptm25";
int w = 1280;
int h = 768;
int sampling = 420;


class blend_node : public media_node {
public:
	media_node_input<2, rgb_color> input1;
	media_node_input<2, rgb_color> input2;
	media_node_output<2, rgb_color> output;
	
	blend_node() :
		media_node(1), input1(*this), input2(*this), output(*this) { }

protected:
	void setup_() override {
		output.define_frame_shape(input1.frame_shape());
	}
	
	void process_() override {
		std::transform(
			input1.view().begin(),
			input1.view().end(),
			input2.view().begin(),
			output.view().begin(),
			[&](const rgb_color& p1, const rgb_color& p2) -> rgb_color {
				double t = current_time() / 120.0;
				std::uint8_t r = (1.0-t)*p1.r + t*p2.r;
				std::uint8_t g = (1.0-t)*p1.g + t*p2.g;
				std::uint8_t b = (1.0-t)*p1.b + t*p2.b;
				return { r, g, b };
			}
		);
	}
};


int main() {
	camera_array cams(cams_file);
	projection_camera cam1 = cams[cam1_name];
	projection_camera cam2 = cams[cam2_name];
	
	media_graph graph;
	auto& im1_source = graph.add_node<yuv_file_source>(im1, w, h, sampling);
	auto& im2_source = graph.add_node<yuv_file_source>(im2, w, h, sampling);
	auto& im1_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& im2_converter = graph.add_node<color_converter_node<ycbcr_color, rgb_color>>();
	auto& blend = graph.add_node<blend_node>();
	auto& sink = graph.add_sink<video_file_sink>("output/video.avi");
	im1_converter.input.connect(im1_source.output);
	im2_converter.input.connect(im2_source.output);
	blend.input1.connect(im1_converter.output);
	blend.input2.connect(im2_converter.output);
	sink.input.connect(blend.output);
	graph.setup();
	graph.run();
}
