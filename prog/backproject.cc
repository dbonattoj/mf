#include "../src/io/yuv_importer.h"
#include "../src/io/ply_exporter.h"
#include "../src/ndarray/ndarray_view.h"
#include "../src/ndarray/ndarray.h"
#include "../src/point_cloud/point.h"
#include "../src/point_cloud/point_cloud.h"
#include "../src/config/camera_array.h"
#include "../src/camera/projection_image_camera.h"
#include "../src/color.h"

#include <string>
#include <iostream>

using namespace mf;


std::string dir = "/data/test_sequences/bbb_flowers_noblur/";

std::string im = dir + "BBB_Flowers_cam0025.yuv";
std::string di = dir + "BBB_Flowers_1280x768_Depth_8bps_cf420_cam0025.yuv";
std::string cams_file = dir + "BBBFlowersNoBlur_Physical_and_Virtual_View_camera_parameters.txt";
std::string cam_name = "param_ptm25";

std::string pc = "output.ply";

int w = 1280;
int h = 768;
int sampling = 420;


int main() {	
	ndarray<2, point_xyzrgb> output_pc(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_im(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_di(make_ndsize(h, w));

	std::cout << "load camera" << std::endl;
	camera_array cams(cams_file);
	projection_image_camera cam(cams[cam_name], false, h, w);

	std::cout << "set up importers" << std::endl;
	yuv_importer im_importer(im, make_ndsize(h, w), sampling);
	yuv_importer di_importer(di, make_ndsize(h, w), sampling);
	
	std::cout << "read first frame" << std::endl;
	im_importer.read_frame(input_im);
	di_importer.read_frame(input_di);
	

	std::cout << "backprojecting" << std::endl;
	for(std::ptrdiff_t y = 0; y < h; ++y)
	for(std::ptrdiff_t x = 0; x < w; ++x) {
		auto pix_c = make_ndptrdiff(y, x);
		
		std::uint8_t d_label = input_di.view().at(pix_c).y;
		rgb_color col = color_convert<rgb_color>(input_im.view().at(pix_c));
				
		float z_near = 0.2, z_far = 700.0;
		float d = d_label / 255.0;
		
		float z = 1.0 / ( d/z_near + (1.0-d)/z_far );		
		Eigen_vec3 world_c = cam.point_with_projected_depth(pix_c, z);
		
		point_xyzrgb pt = make_elem_tuple<point_xyz, rgb_color>(world_c, col);
		output_pc.view().at(pix_c) = pt;
	}
	
	std::cout << "exporting ply" << std::endl;
	ply_exporter exporter(pc);
	exporter.write(output_pc.view().reshape(make_ndsize(h * w)));
	
	return 0;
}
