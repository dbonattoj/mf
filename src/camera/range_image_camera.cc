#include "range_image_camera.h"

namespace mf {

range_image_camera::range_image_camera(const pose& ps, const angle_pair& x_limits, const angle_pair& y_limits, std::size_t imw, std::size_t imh) :
	range_camera(ps, x_limits, y_limits),
	image_camera(imw, imh) { }


range_image_camera::range_image_camera(const pose& ps, angle width, angle height, std::size_t imw, std::size_t imh) :
	range_camera(ps, width, height),
	image_camera(imw, imh) { }	

angle range_image_camera::angular_resolution_x() const {
	return field_of_view_width() / image_width_;
}

angle range_image_camera::angular_resolution_y() const {
	return field_of_view_height() / image_height_;
}

auto range_image_camera::to_image(const Eigen::Vector3f& p) const -> coordinates_type {
	spherical_coordinates s = to_spherical(p);
	s.azimuth -= azimuth_limits_.first;
	s.elevation -= elevation_limits_.first;
	std::ptrdiff_t x = (s.azimuth * image_width_) / field_of_view_width();
	std::ptrdiff_t y = (s.elevation * image_height_) / field_of_view_height();
	return {x, y};
}

Eigen::Vector3f range_image_camera::point(coordinates_type im, float depth) const {
	spherical_coordinates s;
	s.azimuth = azimuth_limits_.first + ((field_of_view_width() * im[0]) / image_width_);
	s.elevation = elevation_limits_.first + ((field_of_view_height() * im[1]) / image_height_);
	s.radius = depth;
	return camera::point(s);
}


}
