#include "yuv_importer.h"
#include "../utility/io.h"

namespace mf {

yuv_importer::yuv_importer(const std::string& filename, const ndsize<2>& frame_shape, int sampling) :
	base(frame_shape),
	file_(filename, std::ios_base::in | std::ios_base::binary)
{
	file_.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
	switch(sampling) {
		case 444: chroma_scale_x_ = 1; chroma_scale_y_ = 1; break;
		case 420: chroma_scale_x_ = 2; chroma_scale_y_ = 2; break;
		default: throw std::invalid_argument("unknown YUV file chroma sampling format");
	}
	
	std::streamsize luma_size = frame_shape[1] * frame_shape[0];
	std::streamsize chroma_size = (frame_shape[1] / chroma_scale_x_) * (frame_shape[0] / chroma_scale_y_);
	frame_size_ = luma_size + 2*chroma_size;

	// allocate buffer for raw frame data
	frame_buffer_.reset(new char_type[frame_size_]);

	// make 2D views for Y, Cr, Cb in local buffer
	y_view_.reset(
		frame_buffer_.get(),
		make_ndsize(frame_shape[0], frame_shape[1])
	);
	cb_view_.reset(
		frame_buffer_.get() + luma_size,
		make_ndsize(frame_shape[0] / chroma_scale_y_, frame_shape[1] / chroma_scale_x_)
	);
	cr_view_.reset(
		frame_buffer_.get() + luma_size + chroma_size,
		make_ndsize(frame_shape[0] / chroma_scale_y_, frame_shape[1] / chroma_scale_x_)
	);
}


void yuv_importer::read_frame(const ndarray_view<2, ycbcr_color>& out) {
	const ndsize<2>& shape = base::frame_shape();
	
	if(out.shape() != shape) throw std::invalid_argument("output view has wrong shape");
	
	// read raw frame into local buffer
	file_.read(frame_buffer_.get(), frame_size_);
	
	// copy from 2D views into output
	for(std::ptrdiff_t y = 0; y < shape[0]; ++y)
	for(std::ptrdiff_t x = 0; x < shape[1]; ++x) {
		ycbcr_color& col = out[y][x];
		col.y = y_view_[y][x];
		col.cb = cb_view_[y / chroma_scale_y_][x / chroma_scale_x_];
		col.cr = cr_view_[y / chroma_scale_y_][x / chroma_scale_x_];
	}
	
	reached_end_ = (file_.peek() == -1);
}


}
