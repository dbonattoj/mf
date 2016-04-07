#include "projection_camera.h"
#include <stdexcept>
#include <cmath>
#include <tuple>

namespace mf {


auto projection_camera::read_intrinsic_matrix_
(const Eigen_mat3& intrinsic_mat, const depth_projection_parameters& dpar) -> intrinsic_matrix_result {
	// intrinsic matrix must be of the form
	//   [ sx  0   tx ]
	//   [ 0   sy  ty ]
	//   [ 0   0   1  ]
	// mapping from view space (X,Y,Z) to image space (x,y)
	// x = sx*X/Z + tx
	// y = sy*Y/Z + ty

	if(intrinsic_mat(2, 0) != 0.0 || intrinsic_mat(2, 1) != 0.0 || intrinsic_mat(2, 2) != 1.0
	|| intrinsic_mat(0, 1) != 0.0 || intrinsic_mat(1, 0) != 0.0)
		throw std::invalid_argument("invalid intrinsic camera matrix");
	
	real sx = intrinsic_mat(0, 0), sy = intrinsic_mat(1, 1), tx = intrinsic_mat(0, 2), ty = intrinsic_mat(1, 2);
	
	// creating frustum for symmetric perspective projection, disregarding offset (tx,ty)
	real near_width = 2.0 * dpar.z_near / sx;
	real near_height = 2.0 * dpar.z_near / sy;
	auto projection_fr = projection_view_frustum::symmetric_perspective(near_width, near_height, dpar);
	Eigen_mat4 fr_projection_mat = projection_fr.projection_transformation().matrix();


	// form projection matrix (with offset)
	Eigen_mat4 projection_mat = fr_projection_mat;
	projection_mat(0, 0) = sx; // ...theoretically same, but take originals to avoid fp precision loss
	projection_mat(1, 1) = sy; // ...

	projection_mat(0, 2) += tx;
	projection_mat(1, 2) += ty;
	
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
	
	transformation_ = Eigen_projective3(mat) * ps.transformation_from_world();
	inverse_transformation_ = transformation_.inverse();
}


projection_camera::projection_camera(const pose& ps, const intrinsic_matrix_result& par) :
	depth_camera(ps),
	projection_frustum_(par.first)
{
	transformation_ = par.second * ps.transformation_from_world();
	inverse_transformation_ = transformation_.inverse();
}
	


projection_camera::projection_camera(const pose& ps, const Eigen_mat3& mat, const depth_projection_parameters& dpar) :
	projection_camera(ps, read_intrinsic_matrix_(mat, dpar)) { }

}
