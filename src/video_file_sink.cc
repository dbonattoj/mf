#include "video_file_sink.h"
#include "opencv.h"
#include "opencv_ndarray.h"

namespace mf {

video_file_sink::video_file_sink(const std::string& filename, const ndsize<2>& size) {
	register_input_(input);
	writer_.open(
		filename,
		CV_FOURCC('m', 'p', '4', 'v'),
		20.0,
		cv::Size(size[1], size[0]),
		true
	);
}
	
void video_file_sink::process_() {
	auto mat = to_opencv_mat(input.view()[0]);
	writer_ << mat;
}


}
