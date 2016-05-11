#ifndef PROG_INPUT_DATA_H_
#define PROG_INPUT_DATA_H_

#include <cstdint>
#include <vector>
#include <mf/camera/projection_image_camera.h>
#include <mf/config/vsrs_camera_array.h>

using camera_type = mf::projection_image_camera<std::uint8_t>;

struct input_visual {
	camera_type camera;
	std::string image_yuv_file;
	std::string depth_image_yuv_file;
};

struct input_data {
	std::size_t image_width;
	std::size_t image_height;
	int yuv_sampling;
	mf::depth_projection_parameters projection_parameters;
	
	std::vector<input_visual> visuals;
};

input_data poznan_blocks();

#endif
