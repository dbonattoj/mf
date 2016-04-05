#include "image_camera.h"

namespace mf {

image_camera::image_camera(const ndsize<2>& sz) :
	image_size_(sz) { }


real image_camera::image_aspect_ratio() const {
	return static_cast<real>(image_width_) / static_cast<real>(image_height_);
}


void image_camera::set_image_width(std::size_t imw) {
	image_size_[1] = imw / image_aspect_ratio();
	image_size_[0] = imw;
}


void image_camera::set_image_height(std::size_t imh) {
	image_size_[0] = image_aspect_ratio() * imh;
	image_size_[1] = imh;
}


void image_camera::flip_pixel_coordinates() {
	image_size_ = flip(image_size_);
	flipped_ = ! flipped_;
}



}
