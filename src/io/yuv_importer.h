/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_YUV_IMPORTER_H_
#define MF_YUV_IMPORTER_H_

#include <fstream>
#include <memory>
#include "../color.h"
#include "../io/seekable_frame_importer.h"
#include "../ndarray/ndarray_view.h"

namespace mf {

// TODO rewrite with raw_video_frame_format, name raw_video_importer

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
