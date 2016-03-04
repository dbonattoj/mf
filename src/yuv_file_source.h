#ifndef MF_YUV_FILE_SOURCE_H_
#define MF_YUV_FILE_SOURCE_H_

#include <string>
#include <fstream>
#include <memory>
#include <cstdint>
#include "node.h"
#include "color.h"

namespace mf {

class yuv_file_source : public node<2, ycbcr_color> {
	using base = node<2, ycbcr_color>;

private:
	std::ifstream file_;
	std::size_t width_;
	std::size_t height_;
	std::size_t chroma_scale_x_;
	std::size_t chroma_scale_y_;
		
	void read_frame_(const ndarray_view<2, ycbcr_color>&);

protected:
	void process_frame_(const ndarray_view<2, ycbcr_color>&) override;

public:
	yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling);
};

}

#endif
