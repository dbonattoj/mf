#include <mf/io/yuv_importer.h>
#include <mf/io/ply_exporter.h>
#include <mf/ndarray/ndarray_view.h>
#include <mf/ndarray/ndarray.h>
#include <mf/point_cloud/point.h>
#include <mf/point_cloud/point_cloud.h>
#include <mf/config/tanimoto_camera_array.h>
#include <mf/camera/projection_image_camera.h>
#include <mf/color.h>

#include <string>
#include <iostream>

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
std::string out = "output/video.avi";

std::string pc = "output/output.ply";

int frame = 25;


void add(std::string I, ply_exporter& exporter) {
	std::string im1 = dir + im+I+".yuv";
	std::string di1 = dir + di+I+".yuv";
	std::string cams_file = dir + cm;
	std::string cam_name = pr+I;


	ndarray<2, point_xyzrgb> output_pc(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_im(make_ndsize(h, w));
	ndarray<2, ycbcr_color> input_di(make_ndsize(h, w));

	std::cout << "load camera" << std::endl;
	depth_projection_parameters dparam;
	dparam.z_near = z_near;
	dparam.z_far = z_far;
	dparam.flip_z = false;
	dparam.range = depth_projection_parameters::unsigned_normalized_disparity;
	tanimoto_camera_array cams(cams_file, dparam, {w, h});
	projection_image_camera<std::uint8_t> cam(cams[cam_name], make_ndsize(w, h));
	cam.flip_pixel_coordinates();

	std::cout << "set up importers" << std::endl;
	yuv_importer im_importer(im1, make_ndsize(h, w), sampling);
	yuv_importer di_importer(di1, make_ndsize(h, w), sampling);
	
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
		
		if(d < 0.3) d = 1.0;
		
		auto c = cam.to_image(pix_c);
		
		Eigen_vec3 world_c = cam.point(c, d);
				
		point_xyzrgb pt = make_elem_tuple<point_xyz, rgb_color>(world_c, col);
		output_pc.view().at(pix_c) = pt;
	}
	
	std::cout << "exporting ply" << std::endl;
	exporter.write(output_pc.view().reshape(make_ndsize(h * w)));
}

int main() {
	ply_exporter exporter(pc);
	for(std::string I : {"2", "3", "4", "5"}) add(I, exporter);
}
