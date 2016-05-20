/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "video_exporter.h"
#include "../opencv.h"
#include "../ndarray/ndarray.h"

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
	auto mat = to_opencv_mat(vw).clone();
	cv::cvtColor(mat, mat, CV_RGB2BGR);
	writer_.write(mat);
}

void video_exporter::close() {
	writer_.release();
}

}

