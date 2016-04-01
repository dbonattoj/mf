#include "video_exporter.h"
#include "../image/image.h"
#include "../opencv_ndarray.h"
#include <iostream>

namespace mf {

video_exporter::video_exporter
(const std::string& filename, const ndsize<2>& frame_shape, float frame_rate, const std::string& format) :
	base(frame_shape)
{
	writer_.open(
		filename,
		CV_FOURCC(format[0], format[1], format[2], format[3]),
		frame_rate,
		cv::Size(frame_shape[1], frame_shape[0]), // TODO ndcoord -> cv::Size cast
		true
	);
}

void video_exporter::write_frame(const ndarray_view<2, rgb_color>& vw) {
	image<rgb_color> img(vw);
	writer_.write(img.cv_mat());
}

void video_exporter::close() {
	writer_.release();
}

}

