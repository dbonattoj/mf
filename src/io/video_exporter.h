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

#ifndef MF_VIDEO_EXPORTER_H_
#define MF_VIDEO_EXPORTER_H_

#include <string>
#include "frame_exporter.h"
#include "../color.h"
#include "../opencv.h"

namespace mf {

/// Frame exporter which writes video file.
/** Uses OpenCV `VideoWriter`. */
class video_exporter : public frame_exporter<2, rgb_color> {
	using base = frame_exporter<2, rgb_color>;

private:
	cv::VideoWriter writer_;

public:
	video_exporter(
		const std::string& filename,
		const ndsize<2>& frame_shape,
		float frame_rate = 25.0,
		const std::string& format = "mp4v"
	);

	void write_frame(const ndarray_view<2, rgb_color>&) override;
	void close() override;
};

}

#endif
