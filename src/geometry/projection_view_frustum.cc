#include "projection_frustum.h"
#include <array>
#include <cmath>
#include <stdexcept>

#include "math_constants.h"
#include "../eigen.h"

namespace mf {


std::array<real, 2> projection_view_frustum::depth_components_(const depth_projection_parameters& dparam) {
	real z_diff = dparam.z_far - dparam.z_near;
	std::array<real, 3> components;
	
	if(dparam.depth == depth_projection_parameters::signed_normalized) {
		components[0] = (dparam.z_far + dparam.z_near)/z_diff;
		components[1] = -(2.0 * dparam.z_far * dparam.z_near)/z_diff;
	} else {
		components[0] = dparam.z_far/zdiff;
		components[1] = -(dparam.z_far * dparam.z_near)/z_diff;
	}
	
	components[3] = 1.0;	
	if(dparam.flip_z) {
		components[0] *= -1.0;
		components[3] *= -1.0;
	}
	
	return components;
}	


projection_view_frustum projection_view_frustum::symmetric_perspective
(real near_width, real near_height, const depth_projection_parameters& dparam) {
	real two_nz = 2.0 * dparam.z_near;
	real x_diff = near_width;
	real y_diff = near_height;

	auto dcomp = depth_components_(dparam);

	Eigen_mat4 mat; mat <<
		two_nz / near_width, 0.0, 0.0, 0.0,
		0.0, two_nz / near_height, 0.0, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;
				
	return projection_frustum(mat, dparam);
}


projection_view_frustum projection_view_frustum::symmetric_perspective_fov
(angle near_width, angle near_height, const depth_projection_parameters& dparam) {
	real z_diff = dparam.z_far - dparam.z_near;
	auto dcomp = depth_components_(dparam);

	Eigen_mat4 mat; mat <<
		std::tan(half_pi - near_width), 0.0, 0.0, 0.0,
		0.0, std::tan(half_pi - near_height), 0.0, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;
				
	return projection_frustum(mat, dparam);
}


projection_view_frustum projection_view_frustum::symmetric_perspective_fov_x
(angle near_width, real aspect_ratio, const depth_projection_parameters& dparam) {
	real w = std::tan(half_pi - near_width);
	real h = w / aspect_ratio;
	auto dcomp = depth_components_(dparam);

	Eigen_mat4 mat; mat <<
		w, 0.0, 0.0, 0.0,
		0.0, h, 0.0, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;
				
	return projection_frustum(mat, dparam);
}



projection_view_frustum projection_view_frustum::symmetric_perspective_fov_y
(angle near_height, real aspect_ratio, const depth_projection_parameters& dparam) {
	real h = std::tan(half_pi - near_height);
	real w = h * aspect_ratio;
	auto dcomp = depth_components_(dparam);

	Eigen_mat4 mat; mat <<
		w, 0.0, 0.0, 0.0,
		0.0, h, 0.0, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;
				
	return projection_frustum(mat, dparam);
}



projection_view_frustum projection_view_frustum::asymmetric_perspective
(distance_pair near_x, distance_pair near_y, const depth_projection_parameters& dparam) {
	real two_nz = 2.0 * dparam.z_near;
	real x_diff = near_x.second - near_x.first;
	real y_diff = near_y.second - near_y.first;
	real z_diff = dparam.z_far - dparam.z_near;
	auto dcomp = depth_components_(dparam);

	Eigen_mat4 mat; mat <<
		two_nz / x_diff, 0.0, (near_x.first + near_x.second)/x_diff, 0.0,
		0.0, two_nz / y_diff, (near_y.first + near_y.second)/y_diff, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;
				
	return projection_frustum(mat, dparam);
}


projection_view_frustum projection_view_frustum::asymmetric_perspective_fov
(angle_pair near_x, angle_pair near_y, const depth_projection_parameters& dparam) {
	angle_pair near_x_proj { std::tan(near_x.first) * dparam.z_near, std::tan(near_x.second) * dparam.z_near };
	angle_pair near_y_proj { std::tan(near_y.first) * dparam.z_near, std::tan(near_y.second) * dparam.z_near };
	return asymmetric_perspective(near_x_proj, near_y_proj, dparam);
}



real projection_view_frustum::aspect_ratio() const {
	return matrix_()(1, 1) / projection_matrix()(0, 0);
}


bool projection_view_frustum::is_symmetric_x() const {
	return (matrix_()(0, 2) == 0.0);
}


bool projection_view_frustum::is_symmetric_y() const {
	return (matrix_()(1, 2) == 0.0);
}


bool projection_view_frustum::is_symmetric() const {
	return is_symmetric_x() && is_symmetric_y();
}


void projection_view_frustum::adjust_fov_x_to_aspect_ratio(real aspect_ratio) {
	if(! is_symmetric()) throw std::logic_error("cannot adjust frustum to image size when it is not symmetric");
	matrix_()(0, 0) = matrix_()(1, 1) / aspect_ratio;
}

void projection_view_frustum::adjust_fov_y_to_aspect_ratio(real aspect_ratio) {
	if(! is_symmetric()) throw std::logic_error("cannot adjust frustum to image size when it is not symmetric");
	matrix_()(1, 1) = matrix_()(0, 0) * aspect_ratio;
}



}
