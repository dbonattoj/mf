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
