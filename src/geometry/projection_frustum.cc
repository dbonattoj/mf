#include "projection_frustum.h"
#include <array>
#include <cmath>
#include "../eigen.h"

namespace mf {


const float projection_frustum::default_z_near_ = 0.01;
const float projection_frustum::default_z_far_ = 1000.0;

projection_frustum::projection_frustum(const Eigen::Matrix4f& mat) :
	frustum(mat) { }


projection_frustum projection_frustum::symmetric_perspective
(float near_width, float near_height, float near_z, float far_z) {
	float dz = far_z - near_z;	

	Eigen::Matrix4f projection_matrix;
	projection_matrix <<
		near_z / near_width, 0, 0, 0,
		0, near_z / near_height, 0, 0,
		0, 0, -(far_z + near_z)/dz, (-2.0f*near_z*far_z)/dz,
		0, 0, -1, 0;
				
	return projection_frustum(projection_matrix);
}


projection_frustum projection_frustum::symmetric_perspective_fov
(angle near_width, angle near_height, float near_z, float far_z) {
	float dz = far_z - near_z;	

	Eigen::Matrix4f projection_matrix;
	projection_matrix <<
		1.0 / std::tan(near_width / 2.0f), 0, 0, 0,
		0, 1.0 / std::tan(near_height / 2.0f), 0, 0,
		0, 0, -(far_z + near_z)/dz, (-2.0f*near_z*far_z)/dz,
		0, 0, -1, 0;
				
	return projection_frustum(projection_matrix);
}


projection_frustum projection_frustum::symmetric_perspective_fov_x
(angle near_width, float aspect_ratio, float near_z, float far_z) {
	float dz = far_z - near_z;
	float tx = std::tan(near_width / 2.0f);

	Eigen::Matrix4f projection_matrix;
	projection_matrix <<
		1.0 / tx, 0, 0, 0,
		0, 1.0 / (tx * aspect_ratio), 0, 0,
		0, 0, -(far_z + near_z)/dz, (-2.0f*near_z*far_z)/dz,
		0, 0, -1, 0;
				
	return projection_frustum(projection_matrix);
}



projection_frustum projection_frustum::symmetric_perspective_fov_y
(angle near_height, float aspect_ratio, float near_z, float far_z) {
	float dz = far_z - near_z;
	float ty = std::tan(near_height / 2.0f);

	Eigen::Matrix4f projection_matrix;
	projection_matrix <<
		aspect_ratio / ty, 0, 0, 0,
		0, 1.0 / ty, 0, 0,
		0, 0, -(far_z + near_z)/dz, (-2.0f*near_z*far_z)/dz,
		0, 0, -1, 0;
				
	return projection_frustum(projection_matrix);
}



projection_frustum projection_frustum::asymmetric_perspective
(distance_pair near_x, distance_pair near_y, float near_z, float far_z) {
	float dz = far_z - near_z;	
	float w = near_x.second - near_x.first;
	float h = near_y.second - near_y.first;
	float two_nz = 2.0f * near_z;

	Eigen::Matrix4f projection_matrix;
	projection_matrix <<
		two_nz / w, 0, (near_x.first + near_x.second)/w, 0,
		0, two_nz / h, (near_y.first + near_y.second)/h, 0,
		0, 0, -(far_z + near_z)/dz, (-2.0f*near_z*far_z)/dz,
		0, 0, -1, 0;
				
	return projection_frustum(projection_matrix);
}


projection_frustum projection_frustum::asymmetric_perspective_fov
(angle_pair near_x, angle_pair near_y, float near_z, float far_z) {
	angle_pair near_x_proj { std::tan(near_x.first) * near_z, std::tan(near_x.second) * near_z };
	angle_pair near_y_proj { std::tan(near_y.first) * near_z, std::tan(near_y.second) * near_z };
	return asymmetric_perspective(near_x_proj, near_y_proj, near_z, far_z);
}


float projection_frustum::aspect_ratio() const {
	return matrix(1, 1) / matrix(0, 0);
}


bool projection_frustum::is_symmetric_x() const {
	return (matrix(0, 2) == 0);
}


bool projection_frustum::is_symmetric_y() const {
	return (matrix(1, 2) == 0);
}


bool projection_frustum::is_symmetric() const {
	return is_symmetric_x() && is_symmetric_y();
}


float projection_frustum::near_z() const {
	return matrix(2, 3) / (matrix(2, 2) - 1);
}


float projection_frustum::far_z() const {
	return matrix(2, 3) / (matrix(2, 2) + 1);
}


void projection_frustum::adjust_fov_x_to_aspect_ratio(float aspect_ratio) {
	if(! is_symmetric()) throw std::logic_error("cannot adjust frustum to image size when it is not symmetric");
	matrix(0, 0) = matrix(1, 1) / aspect_ratio;
}

void projection_frustum::adjust_fov_y_to_aspect_ratio(float aspect_ratio) {
	if(! is_symmetric()) throw std::logic_error("cannot adjust frustum to image size when it is not symmetric");
	matrix(1, 1) = matrix(0, 0) * aspect_ratio;
}



}
