#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include "depth_camera.h"
#include "../geometry/projection_view_frustum.h"
#include <utility>

namespace mf {

/// Pin-hole camera with projection to a planar image space.
/** Defined using `projection_view_frustum`. Image coordinates for points within frustum are in range `[-1, +1]`. */
class projection_camera : public depth_camera {
public:
	struct image_parameters {
		Eigen_vec2 scale; ///< Projected image coordinates in frustum are scaled from `[-1, +1]` to `[-scale, +scale]`.
		Eigen_vec2 offset; ///< After scaling, maps coordinates to `[-scale+offset, +scale+offset]`.
	};
	
public:
	using intrinsic_matrix_result = std::pair<projection_view_frustum, Eigen_projective3>;

	projection_view_frustum projection_frustum_; ///< Projection frustum, without pose and without image scale, offset.
	Eigen_projective3 transformation_;
	Eigen_projective3 inverse_transformation_;
	
	static intrinsic_matrix_result read_intrinsic_matrix_
		(const Eigen_mat3& intrinsic_matrix, const depth_projection_parameters&);

	projection_camera(const pose&, const intrinsic_matrix_result&);
	
public:		
	projection_camera(const pose&, const projection_view_frustum&, const image_parameters&);
	
	projection_camera(const pose&, const Eigen_mat3& intrinsic_matrix, const depth_projection_parameters&);
	
	const projection_view_frustum& relative_frustum() const { return projection_frustum_; }
	const depth_projection_parameters& depth_parameters() const { return projection_frustum_.depth_parameters(); }

	image_coordinates_type project(const Eigen_vec3& p) const override {
		return (transformation_ * p.homogeneous()).eval().hnormalized().head(2);
	}
	
	real depth(const Eigen_vec3& p) const override {
		return (transformation_ * p.homogeneous()).eval().hnormalized()[2];
	}

	Eigen_vec3 ray_direction(const image_coordinates_type& c) const override {
		real d = 1.0;
		Eigen_vec3 p(c[0], c[1], d);
		return (inverse_transformation_ * p.homogeneous()).eval().hnormalized().normalized();
	}
	
	Eigen_vec3 point(const image_coordinates_type& c, real depth) const override {
		Eigen_vec3 p(c[0], c[1], depth);
		return (inverse_transformation_ * p.homogeneous()).eval().hnormalized();
	}
};



}

#endif
