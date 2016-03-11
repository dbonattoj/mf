#ifndef MF_VIDEO_FILE_SINK_H_
#define MF_VIDEO_FILE_SINK_H_

#include <string>
#include "opencv.h"
#include "color.h"
#include "media_node.h"
#include "ndcoord.h"

namespace mf {

class video_file_sink : public media_node_sink {
	cv::VideoWriter writer_;
	
public:
	media_node_input<2, rgb_color> input;

	video_file_sink(const std::string& filename, const ndsize<2>& size);
	
	void process_() override;
};

}

#endif
