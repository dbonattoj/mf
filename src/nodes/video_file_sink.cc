#include "video_file_sink.h"
#include "../opencv.h"
#include "../opencv_ndarray.h"
#include "../image/image.h"
#include <iostream>

namespace mf {

void video_file_sink::setup_() {
	const auto& size = input.frame_shape();
	writer_.open(
		filename_,
		CV_FOURCC('m', 'p', '4', 'v'),
		25.0,
		cv::Size(size[1], size[0]),
		true
	);
}
	

void video_file_sink::process_() {
	image<rgb_color> img(input.view());
	writer_ << img.cv_mat();
}


}
