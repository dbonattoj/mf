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

#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include "depth_camera.h"
#include "../geometry/projection_view_frustum.h"
#include "../ndarray/ndcoord.h"
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
	using intrinsic_matrix_result = std::pair<projection_view_frustum, Eigen_projective3>;

	projection_view_frustum projection_frustum_; ///< Projection frustum, without pose and without image scale, offset.
	Eigen_projective3 view_to_image_; ///< Transformation from view to image. Without pose, with image scale and offset.
	Eigen_projective3 world_to_image_; ///< Full transformation from world to image. Pose, projection, scale, offset.
	Eigen_projective3 image_to_world_; ///< Inverse of `world_to_image_`.
	
	static intrinsic_matrix_result read_intrinsic_matrix_
		(const Eigen_mat3& intrinsic_matrix, const depth_projection_parameters&, const ndsize<2>&);

	projection_camera(const pose&, const intrinsic_matrix_result&);

protected:
	void do_update_pose() override;

public:		
	projection_camera(const pose&, const projection_view_frustum&, const image_parameters&);
	projection_camera(const pose&, const Eigen_mat3& intrinsic, const depth_projection_parameters&, const ndsize<2>&);
	
	projection_camera(const projection_camera&) = default;
	projection_camera& operator=(const projection_camera&) = default;
	
	const projection_view_frustum& relative_frustum() const { return projection_frustum_; }
	const depth_projection_parameters& depth_parameters() const { return projection_frustum_.depth_parameters(); }

	image_coordinates_type project(const Eigen_vec3& p) const override {
		return (world_to_image_ * p.homogeneous()).eval().hnormalized().head(2);
	}
	
	real depth(const Eigen_vec3& p) const override {
		return (world_to_image_ * p.homogeneous()).eval().hnormalized()[2];
	}

	Eigen_vec3 ray_direction(const image_coordinates_type& c) const override {
		real depth = 1.0;
		Eigen_vec3 p(c[0], c[1], depth);
		return (image_to_world_ * p.homogeneous()).eval().hnormalized().normalized();
	}
	
	Eigen_vec3 point(const image_coordinates_type& c, real depth) const override {
		Eigen_vec3 p(c[0], c[1], depth);
		return (image_to_world_ * p.homogeneous()).eval().hnormalized();
	}
	
	Eigen_mat3 intrinsic_matrix() const;
	const Eigen_mat4& intrinsic_matrix_with_depth() const { return view_to_image_.matrix(); }
	
	const Eigen_projective3& world_to_image_transformation() const { return world_to_image_; }
	const Eigen_projective3& image_to_world_transformation() const { return image_to_world_; }
};


Eigen_projective3 homography_transformation(const projection_camera& from, const projection_camera& to);

Eigen_mat3 fundamental_matrix(const projection_camera& from, const projection_camera& to);

}

#endif
