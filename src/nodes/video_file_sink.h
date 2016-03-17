#ifndef MF_VIDEO_FILE_SINK_H_
#define MF_VIDEO_FILE_SINK_H_

#include <string>
#include "../graph/media_sink_node.h"
#include "../graph/media_node_input.h"
#include "../opencv.h"
#include "../color.h"
#include "../ndarray/ndcoord.h"

namespace mf {

class video_file_sink : public media_sink_node {
	std::string filename_;
	cv::VideoWriter writer_;
	
public:
	media_node_input<2, rgb_color> input;

	explicit video_file_sink(const std::string& filename) :
		filename_(filename), input(*this) { }
	
	void setup_() override;
	void process_() override;
};

}

#endif
