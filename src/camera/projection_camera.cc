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
		throw std::invalid_argument("invalid intrinsic camera matrix");
	
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

std::cout << "matrix: \n" << projection_mat << "\n--------------------\n";

	
	return std::make_pair(projection_fr, Eigen_projective3(projection_mat));

	// TODO verify for flip_z
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
	
	world_to_image_ = Eigen_projective3(mat) * ps.transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
}


projection_camera::projection_camera(const pose& ps, const intrinsic_matrix_result& par) :
	depth_camera(ps),
	projection_frustum_(par.first)
{
	world_to_image_ = par.second * ps.transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
}
	


projection_camera::projection_camera
(const pose& ps, const Eigen_mat3& mat, const depth_projection_parameters& dpar, const ndsize<2>& img_sz) :
	projection_camera(ps, read_intrinsic_matrix_(mat, dpar, img_sz)) { }


Eigen_projective3 homography_transformation(const projection_camera& from, const projection_camera& to) {
	return to.world_to_image_ * from.image_to_world_;
}

}
