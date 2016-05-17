#ifndef MF_YUV_IMPORTER_H_
#define MF_YUV_IMPORTER_H_

#include <fstream>
#include <memory>
#include "../color.h"
#include "../io/seekable_frame_importer.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

/// Seekable frame importer which reads YUV file.
class yuv_importer : public seekable_frame_importer<2, ycbcr_color> {
	using base = seekable_frame_importer<2, ycbcr_color>;
	
private:
	using char_type = std::ifstream::char_type;

	std::ifstream file_;
	std::size_t file_size_;
	std::size_t current_time_ = 0;
	
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
	bool reached_end() const override;

	time_unit current_time() const override;
	time_unit total_duration() const override;
	
	void seek(time_unit) override;
};
	
}

#endif
