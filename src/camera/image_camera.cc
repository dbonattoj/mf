#include "image_camera.h"

namespace mf {

image_camera::image_camera(std::size_t imw, std::size_t imh) :
	image_width_(imw), image_height_(imh) { }


float image_camera::image_aspect_ratio() const {
	return static_cast<float>(image_width_) / static_cast<float>(image_height_);
}

void image_camera::set_image_size(ndsize<2> sz) {
	image_width_ = sz[0];
	image_height_ = sz[1];
}

void image_camera::set_image_width(std::size_t imw) {
	image_height_ = static_cast<float>(imw) / image_aspect_ratio();
	image_width_ = imw;
}


void image_camera::set_image_height(std::size_t imh) {
	image_width_ = image_aspect_ratio() * imh;
	image_height_ = imh;
}


std::size_t image_camera::image_number_of_pixels() const {
	return image_width_ * image_height_;
}


bool image_camera::in_bounds(pixel_coordinates_type ic) const {
	return (ic[0] >= 0) && (ic[0] < image_width_) && (ic[1] >= 0) && (ic[1] < image_height_);
}


}
