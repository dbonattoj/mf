#ifndef MF_YUV_FILE_SOURCE_H_
#define MF_YUV_FILE_SOURCE_H_

#include <string>
#include <fstream>
#include "node.h"
#include "color.h"

namespace mf {

class yuv_file_source : public node<2, ycbcr_color> {
	using base = node<2, ycbcr_color>;

private:
	std::ifstream file_;

protected:
	void process_frame_(const ndarray_view<2, ycbcr_color>&) override;

public:
	yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling);
};

}

#endif
