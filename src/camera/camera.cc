#include "camera.h"

namespace mf {

camera::camera(const pose& ps) :
	space_object(ps) { }

	
angle camera::field_of_view_width() const {
	auto limits = this->field_of_view_limits_x();
	return limits.second - limits.first;
}


angle camera::field_of_view_height() const {
	auto limits = this->field_of_view_limits_y();
	return limits.second - limits.first;
}

Eigen::Affine3f camera::view_transformation() const {
	return relative_pose().transformation_from_world();
}

Eigen::Vector3f camera::view_ray_direction() const {
	return relative_pose().orientation * Eigen::Vector3f(0, 0, -1);
}

float camera::depth_sq(const Eigen::Vector3f& p) const {
	return (relative_pose().position - p).squaredNorm();
}

float camera::depth(const Eigen::Vector3f& p) const {
	return (relative_pose().position - p).norm();
}

spherical_coordinates camera::to_spherical(const Eigen::Vector3f& p) const {
	return spherical_coordinates::from_cartesian(view_transformation() * p);
}

Eigen::Vector3f camera::point(const spherical_coordinates& s) const {
	return relative_pose().transformation_to_world() * s.to_cartesian();
}


}
