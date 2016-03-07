#include "yuv_file_source.h"
#include "ndarray_view.h"
#include <iostream>

namespace mf {


void yuv_file_source::read_frame_(const ndarray_view<2, ycbcr_color>& out) {
	// Get sizes of luma, 2 chroma parts, and whole frame
	std::streamsize luma_size = width_ * height_;
	std::streamsize chroma_size = (width_ / chroma_scale_x_) * (height_ / chroma_scale_y_);
	std::streamsize frame_size = luma_size + 2*chroma_size;

	// Read raw frame into local buffer
	using char_type = std::ifstream::char_type;
	std::vector<char_type> buffer(frame_size);
	file_.read(buffer.data(), frame_size);
	
	// Make 2D views for Y, Cr, Cb in local buffer
	ndarray_view<2, char_type> y_view(
		buffer.data(),
		make_ndsize(height_, width_)
	);
	ndarray_view<2, char_type> cb_view(
		buffer.data() + luma_size,
		make_ndsize(height_ / chroma_scale_y_, width_ / chroma_scale_x_)
	);
	ndarray_view<2, char_type> cr_view(
		buffer.data() + luma_size + chroma_size,
		make_ndsize(height_ / chroma_scale_y_, width_ / chroma_scale_x_)
	);
	
	// Copy into out
	std::cout << "h=" << height_ << " w=" << width_ << std::endl; 
	
	for(std::ptrdiff_t y = 0; y < height_; ++y) for(std::ptrdiff_t x = 0; x < width_; ++x) {
		ycbcr_color& col = out[y][x];
		col.y = y_view[y][x];
		col.cb = cb_view[y / chroma_scale_y_][x / chroma_scale_x_];
		col.cr = cr_view[y / chroma_scale_y_][x / chroma_scale_x_];
	}
}


void yuv_file_source::process_frame_() {
	auto sec = output_.write(1);
	read_frame_(sec[0]);
	output_.did_write(1);
}


yuv_file_source::yuv_file_source(const std::string& filename, std::size_t width, std::size_t height, int sampling) :
	base(make_ndsize(height, width)),
	file_(filename, std::ios_base::in | std::ios_base::binary),
	width_(width),
	height_(height)
{
	switch(sampling) {
		case 444: chroma_scale_x_ = 1; chroma_scale_y_ = 1; break;
		case 420: chroma_scale_x_ = 2; chroma_scale_y_ = 2; break;
	}
}

}
