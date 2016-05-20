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
