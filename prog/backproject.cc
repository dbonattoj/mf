#include "../src/io/yuv_importer.h"
#include "../src/io/ply_exporter.h"
#include "../src/ndarray/ndarray_view.h"
#include "../src/ndarray/ndarray.h"
#include "../src/point_cloud/point.h"
#include "../src/point_cloud/point_cloud.h"
#include "../src/config/tanimoto_camera_array.h"
#include "../src/camera/projection_image_camera.h"
#include "../src/color.h"

#include <string>
#include <iostream>

using namespace mf;


std::string dir = "/data/test_sequences/bbb_flowers_noblur/";
//std::string dir = "/Users/timlenertz/Desktop/mf/data/";

std::string im = dir + "BBB_Flowers_cam0035.yuv";
std::string di = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0035.yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam_name = "param_ptm35";
std::string pc = "output/output.ply";

int w = 1280;
int h = 768;
int sampling = 420;
int frame = 25;


int main() {	
	ndarray<2, point_xyzrgb> output_pc(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_im(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_di(make_ndsize(h, w));

	std::cout << "load camera" << std::endl;
	depth_projection_parameters dparam;
	dparam.z_near = 0.2;
	dparam.z_far = 700.0;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;
	tanimoto_camera_array cams(cams_file, dparam);
	projection_image_camera<std::uint8_t> cam(cams[cam_name], make_ndsize(w, h));
	cam.flip_pixel_coordinates();

	std::cout << "set up importers" << std::endl;
	yuv_importer im_importer(im, make_ndsize(h, w), sampling);
	yuv_importer di_importer(di, make_ndsize(h, w), sampling);
	
	std::cout << "read first frame" << std::endl;
	im_importer.read_frame_at(input_im, frame);
	di_importer.read_frame_at(input_di, frame);
	

	std::cout << "backprojecting" << std::endl;

	for(std::ptrdiff_t y = 0; y < h; ++y)
	for(std::ptrdiff_t x = 0; x < w; ++x) {
		auto pix_c = make_ndptrdiff(y, x);
		
		std::uint8_t pix_d = input_di.view().at(pix_c).y;
		rgb_color col = color_convert<rgb_color>(input_im.view().at(pix_c));

		real d = cam.to_depth(pix_d);
		auto c = cam.to_image(pix_c);
		
		Eigen_vec3 world_c = cam.point(c, d);
				
		point_xyzrgb pt = make_elem_tuple<point_xyz, rgb_color>(world_c, col);
		output_pc.view().at(pix_c) = pt;
	}
	
	std::cout << "exporting ply" << std::endl;
	ply_exporter exporter(pc);
	exporter.write(output_pc.view().reshape(make_ndsize(h * w)));
	
	return 0;
}
