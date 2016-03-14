#ifndef MF_YUV_FILE_SOURCE_H_
#define MF_YUV_FILE_SOURCE_H_

#include <string>
#include <fstream>
#include <memory>
#include <cstdint>
#include "media_node.h"
#include "color.h"

namespace mf {

class yuv_file_source : public media_node_source {
private:
	std::ifstream file_;
	std::size_t width_;
	std::size_t height_;
	std::size_t chroma_scale_x_;
	std::size_t chroma_scale_y_;
		
	void read_frame_(const ndarray_view<2, ycbcr_color>&);

protected:
	void process_() override;

public:
	media_node_output<2, ycbcr_color> output;

	yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling);
};

}

#endif
