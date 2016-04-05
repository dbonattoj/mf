#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include "depth_camera.h"
#include "../geometry/projection_frustum.h"
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
	
private:
	projection_view_frustum projection_frustum_;
	Eigen_mat4 view_projection_matrix_;
	Eigen_mat4 inverse_view_projection_matrix_;
	
	static std::pair<projection_view_frustum, Eigen_mat4> read_intrinsic_matrix_
		(const Eigen_mat3& intrinsic_matrix, const projection_depth_parameters&);

	projection_camera(const pose&, const std::pair<projection_view_frustum, Eigen_mat4>&);
	
public:		
	projection_camera(const pose&, const projection_view_frustum&, const image_parameters&);
	
	projection_camera(const pose&, const Eigen_mat3& intrinsic_matrix, const projection_depth_parameters&);
	
	const projection_view_frustum& relative_frustum() const { return projection_frustum_; }
	const projection_depth_parameters& depth_parameters() const { return projection_frustum_.depth_parameters(); }

	image_coordinates project(const Eigen_vec3& p) const override {
		return (view_projection_matrix_ * p.homogeneous()).hnormalized().head(2);
	}
	
	real depth(const Eigen_vec3& p) const override {
		return (view_projection_matrix_ * p.homogeneous()).hormalized()[2];
	}

	Eigen_vec3 ray_direction(const image_coordinates& c) const override {
		return (inverse_view_projection_matrix_ * Eigen_vec4(c[0], c[1], 1.0, 0.0)).hnormalized().normalized();
	}
	
	Eigen_vec3 point(const image_coordinates& c, real depth) const override {
		return (inverse_view_projection_matrix_ * Eigen_vec4(c[0], c[1], depth, 1.0)).hnormalized();
	}
};



}

#endif
