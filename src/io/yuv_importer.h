#ifndef MF_YUV_IMPORTER_H_
#define MF_YUV_IMPORTER_H_

#include <fstream>
#include <memory>
#include "../color.h"
#include "../io/frame_importer.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

class yuv_importer : public frame_importer<2, ycbcr_color> {
	using base = frame_importer<2, ycbcr_color>;
	
private:
	using char_type = std::ifstream::char_type;

	std::ifstream file_;
	bool reached_end_ = false;
	
	std::unique_ptr<char_type[]> frame_buffer_;
	std::streamsize frame_size_;
	ndarray_view<2, char_type> y_view_;
	ndarray_view<2, char_type> cb_view_;
	ndarray_view<2, char_type> cr_view_;
	
	std::size_t chroma_scale_y_;
	std::size_t chroma_scale_x_;
	
public:
	yuv_importer(const std::string& filename, const ndsize<2>& frame_shape, int sampling);
		
	void read_frame(const ndarray_view<2, ycbcr_color>&) override;
	bool reached_end() const override { return reached_end_; }
};
	
}

#endif
