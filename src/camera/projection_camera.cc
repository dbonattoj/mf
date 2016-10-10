#include "projection_camera.h"
#include "../geometry/projection_view_frustum.h"

#include <iostream>

namespace mf {

void projection_camera::update_world_matrices_() {
	world_to_image_ = view_to_image_ * absolute_pose().transformation_from_world();
	image_to_world_ = world_to_image_.inverse();
	
	std::cout << view_to_image_.matrix() << "\n----------" << std::endl;
}


void projection_camera::do_update_pose() {
	update_world_matrices_();
}


void projection_camera::scale(real factor) {
	view_to_image_(0, 0) *= factor;
	view_to_image_(1, 1) *= factor;
	view_to_image_(0, 3) *= factor;
	view_to_image_(1, 3) *= factor;

	update_world_matrices_();
}


Eigen_mat4 projection_camera::projection_matrix_
(const intrinsic_matrix_type& intr, const depth_projection_parameters& dparam) {
	Assert(intr(2, 0) == 0.0);
	Assert(intr(2, 1) == 0.0);
	Assert(intr(2, 2) == 1.0);

	Eigen_mat4 proj; proj <<
		intr(0, 0), intr(0, 1), intr(0, 2), 0.0,
		intr(1, 0), intr(1, 1), intr(1, 2), 0.0,
		0.0, 0.0, dparam.offset(), dparam.factor(),
		0.0, 0.0, 1.0, 0.0;
		
	if(dparam.flip_z) {
		proj(2, 2) = -proj(2, 2);
		proj(3, 2) = -1.0;
	}
	
	return proj;
}


projection_camera::projection_camera
(const pose& ps, const intrinsic_matrix_type& intr, const depth_projection_parameters& dparam) :
	depth_camera(ps),
	depth_parameters_(dparam),
	view_to_image_(projection_matrix_(intr, dparam))
{
	update_world_matrices_();
}
	

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


/*
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
*/

/*
	Eigen_mat4 mat; mat <<
		two_nz / near_width, 0.0, 0.0, 0.0,
		0.0, two_nz / near_height, 0.0, 0.0,
		0.0, 0.0, dcomp[0], dcomp[1],
		0.0, 0.0, dcomp[2], 0.0;

*/
