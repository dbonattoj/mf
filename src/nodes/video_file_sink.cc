#include "video_file_sink.h"
#include "../opencv.h"
#include "../opencv_ndarray.h"

namespace mf {

void video_file_sink::setup_() {
	const auto& size = input.frame_shape();
	writer_.open(
		filename_,
		CV_FOURCC('m', 'p', '4', 'v'),
		20.0,
		cv::Size(size[1], size[0]),
		true
	);
}
	

void video_file_sink::process_() {
	auto mat = to_opencv_mat(input.view());
	writer_ << mat;
}


}
