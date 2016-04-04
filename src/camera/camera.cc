#include "camera.h"

namespace mf {

camera::camera(const pose& ps) :
	space_object(ps) { }

	
Eigen_affine3 camera::view_transformation() const {
	return relative_pose().transformation_from_world();
}

Eigen_vec3 camera::ray_direction(const Eigen_vec3& p) const {
	return relative_pose().orientation * p.normalized();
}

float camera::distance_sq(const Eigen_vec3& p) const {
	return (relative_pose().position - p).squaredNorm();
}

float camera::distance(const Eigen_vec3& p) const {
	return (relative_pose().position - p).norm();
}

spherical_coordinates camera::to_spherical(const Eigen_vec3& p) const {
	return spherical_coordinates::from_cartesian(view_transformation() * p);
}

Eigen_vec3 camera::point(const spherical_coordinates& s) const {
	return relative_pose().transformation_to_world() * s.to_cartesian();
}


}
