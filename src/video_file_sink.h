#ifndef MF_VIDEO_FILE_SINK_H_
#define MF_VIDEO_FILE_SINK_H_

#include <string>
#include "opencv.h"
#include "color.h"
#include "media_node.h"
#include "ndcoord.h"

namespace mf {

class video_file_sink : public media_sink {
private:
	media_node_input<2, rgb_color> image_;
	cv::VideoWriter writer_;
	
public:
	video_file_sink(const std::string& filename, const ndsize<2>& size);
	
	void process_frame_() override;
	
	media_node_input<2, rgb_color>& image_input() { return image_; }
};

}

#endif
