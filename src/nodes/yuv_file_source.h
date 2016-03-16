#ifndef MF_YUV_FILE_SOURCE_H_
#define MF_YUV_FILE_SOURCE_H_

#include <string>
#include <fstream>
#include <memory>
#include <cstdint>
#include "../graph/media_sequential_node.h"
#include "../graph/media_node_output.h"
#include "../color.h"

namespace mf {

class yuv_file_source : public media_sequential_node {
private:
	std::ifstream file_;
	std::size_t width_;
	std::size_t height_;
	std::size_t chroma_scale_x_;
	std::size_t chroma_scale_y_;
		
	void read_frame_(const ndarray_view<2, ycbcr_color>&);

protected:
	void setup_() override;
	void process_() override;
	bool process_reached_end_() const override;

public:
	media_node_output<2, ycbcr_color> output;

	yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling);
};

}

#endif
