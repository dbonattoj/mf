#include "../src/io/yuv_importer.h"
#include "../src/io/video_exporter.h"
#include "../src/ndarray/ndarray_view.h"
#include "../src/ndarray/ndarray.h"
#include "../src/point_cloud/point.h"
#include "../src/point_cloud/point_cloud.h"
#include "../src/config/tanimoto_camera_array.h"
#include "../src/camera/projection_image_camera.h"
#include "../src/color.h"
#include "../src/flow/graph.h"
#include "../src/nodes/importer.h"
#include "../src/nodes/exporter.h"
#include "../src/nodes/color_converter.h"
#include "../src/nodes/blender.h"
#include "../src/nodes/multiwarp.h"
#include <string>

using namespace mf;

std::string dir = "/data/test_sequences/poznan_blocks/";
//std::string dir = "/Users/timlenertz/Desktop/mf/data/";
std::string im = "Poznan_Blocks_1920x1080_texture_cam";
std::string di = "Poznan_Blocks_1920x1080_depth_cf420_8bps_cam";
std::string cm = "Poznan_Blocks_Physical_and_Virtual_View_camera_parameters.txt";
std::string pr = "param_cam";
std::size_t w = 1920, h = 1080;
int sampling = 420;
float z_far = 100.0, z_near = 15.0;
std::string out = "output/video3.avi";


auto shape = make_ndsize(h, w);

int main() {
	flow::graph graph;

	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;

	tanimoto_camera_array cams(dir + cm, dparam, {w, h});
	projection_image_camera<std::uint8_t> vcam(cams[pr + "5"], make_ndsize(w, h));
	vcam.flip_pixel_coordinates();
	auto& wrapper = graph.add_node<node::multiwarp>(vcam);

	for(int i = 2; i <= 8; ++i) {
		std::string cam_name = pr + std::to_string(i);
		projection_image_camera<std::uint8_t> cam(cams[cam_name], make_ndsize(w, h));
		cam.flip_pixel_coordinates();
		
		std::string im_file = dir + im + std::to_string(i) + ".yuv";
		std::string di_file = dir + di + std::to_string(i) + ".yuv";
		
		auto& im_source = graph.add_node<node::seekable_importer<yuv_importer>>(im_file, shape, sampling);
		auto& im_converter = graph.add_node<node::color_converter<ycbcr_color, rgba_color>>();
		auto& di_source = graph.add_node<node::seekable_importer<yuv_importer>>(di_file, shape, sampling);
		auto& di_converter = graph.add_node<node::color_converter<ycbcr_color, std::uint8_t>>();
		im_converter.in.connect(im_source.out);
		di_converter.in.connect(di_source.out);
		
		auto& wr_in = wrapper.add_input_view(cam);
		wr_in.im.connect(im_converter.out);
		wr_in.di.connect(di_converter.out);
	}

	auto& sink_converter = graph.add_node<node::color_converter<rgba_color, rgb_color>>();
	sink_converter.in.connect(wrapper.out);
	auto& sink = graph.add_sink<node::exporter<video_exporter>>(out, shape);
	sink.in.connect(sink_converter.out);
	
	graph.setup();
	graph.run();
}
