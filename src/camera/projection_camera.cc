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

#include "projection_camera.h"
#include <stdexcept>
#include <cmath>
#include <tuple>

#include <iostream>

namespace mf {


auto projection_camera::read_intrinsic_matrix_
(const Eigen_mat3& intrinsic_mat, const depth_projection_parameters& dpar, const ndsize<2>& img_sz) -> intrinsic_matrix_result {
	// intrinsic matrix must be of the form
	//   [ fx  0   tx ]
	//   [ 0   fy  ty ]
	//   [ 0   0   1  ]
	// mapping from view space (X,Y,Z) to image space (x,y)
	// x = fx*X/Z + tx
	// y = fy*Y/Z + ty

	if(intrinsic_mat(2, 0) != 0.0 || intrinsic_mat(2, 1) != 0.0 || intrinsic_mat(2, 2) != 1.0
	|| intrinsic_mat(0, 1) != 0.0 || intrinsic_mat(1, 0) != 0.0)
		throw std::invalid_argument("unsupported intrinsic camera matrix");
	
	real fx = intrinsic_mat(0, 0), fy = intrinsic_mat(1, 1), tx = intrinsic_mat(0, 2), ty = intrinsic_mat(1, 2);
	
	// creating frustum for symmetric perspective projection, disregarding offset (tx,ty)
	// fx, fy includes scaling term to image width/height.
	// for projection frustum, bounds of image coordinates must be [-1, +1]
	real near_width = img_sz[0] * dpar.z_near / fx;
	real near_height = img_sz[1] * dpar.z_near / fy;
	auto projection_fr = projection_view_frustum::symmetric_perspective(near_width, near_height, dpar);
	Eigen_mat4 fr_projection_mat = projection_fr.projection_transformation().matrix();

	// form projection matrix (with offset)
	Eigen_mat4 projection_mat = fr_projection_mat;
	projection_mat(0, 0) = fx;
	projection_mat(1, 1) = fy;

	projection_mat(0, 2) += tx;
	projection_mat(1, 2) += ty;
	
	return std::make_pair(projection_fr, Eigen_projective3(projection_mat));
}


void projection_camera::do_update_pose() {
	world_to_image_ = view_to_image_ * absolute_pose().transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
}


projection_camera::projection_camera(const pose& ps, const projection_view_frustum& fr, const image_parameters& ipar) :
	depth_camera(ps),
	projection_frustum_(fr)
{
	Eigen_mat4 mat = fr.projection_transformation().matrix();
	mat(0, 0) *= ipar.scale[0];
	mat(1, 1) *= ipar.scale[1];
	mat(0, 3) += ipar.offset[0];
	mat(1, 3) += ipar.offset[1];
	
	view_to_image_ = Eigen_projective3(mat);
	
	world_to_image_ = view_to_image_ * ps.transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
}


projection_camera::projection_camera(const pose& ps, const intrinsic_matrix_result& par) :
	depth_camera(ps),
	projection_frustum_(par.first)
{
	view_to_image_ = par.second;
	
	world_to_image_ = view_to_image_ * ps.transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
}
	

projection_camera::projection_camera
(const pose& ps, const Eigen_mat3& mat, const depth_projection_parameters& dpar, const ndsize<2>& img_sz) :
	projection_camera(ps, read_intrinsic_matrix_(mat, dpar, img_sz)) { }


Eigen_mat3 projection_camera::intrinsic_matrix() const {
	const Eigen_mat4& view_to_image = view_to_image_.matrix();
	Eigen_mat3 intrinsic; intrinsic <<
		view_to_image(0, 0), view_to_image(0, 1), view_to_image(0, 2),
		view_to_image(1, 0), view_to_image(1, 1), view_to_image(1, 2),
		view_to_image(3, 0), view_to_image(3, 1), view_to_image(3, 2);
	return intrinsic;
}


Eigen_projective3 homography_transformation(const projection_camera& from, const projection_camera& to) {
	return to.world_to_image_transformation() * from.image_to_world_transformation();
}


Eigen_mat3 fundamental_matrix(const projection_camera& from, const projection_camera& to) {
	return to.intrinsic_matrix().inverse().transpose() * essential_matrix(from, to) * from.intrinsic_matrix().inverse();
}

	
}

