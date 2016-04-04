#ifndef MF_VIEW_FRUSTUM_H_
#define MF_VIEW_FRUSTUM_H_

#include <utility>
#include <array>
#include "angle.h"
#include "../eigen.h"

namespace mf {

struct projection_depth_parameters {
	enum depth_range_convention {
		signed_normal;  ///< Depths of points within frustum are in [-1, +1].
		unsigned_normal; ///< Depths of points within frustum are in [0, 1].
	};

	depth_range_convention depth_range;
	real z_near = 0.1;
	real z_far = 100.0;
};


/// View frustum of a perspective camera, with near and far clipping planes.
/** Represented using 4x4 view-projection matrix, with additional information about convention used. Includes the affine
 ** world to view transformation. Can be constructed only as `projection_view_frustum` (which does not include
 ** this view transformation), and can then be transformed using transform().  */
class view_frustum {
public:	
	enum intersection {
		outside_frustum,
		inside_frustum,
		partially_inside_frustum
	};
	
	using edge = std::pair<Eigen_vec3, Eigen_vec3>;
	using corners_array = std::array<Eigen_vec3, 8>;
	using edges_array = std::array<edge, 12>;
	using planes_array = std::array<Eigen_hyperplane3, 6>;

protected:
	view_frustum(const Eigen_mat4& mat, const projection_depth_parameters& conv);
	view_frustum() = delete;

private:
	Eigen_mat4 view_projection_matrix_;
	projection_depth_parameters depth_range_;

public:
	const Eigen_mat4& view_projection_matrix() const { return view_projection_matrix_; }
	
	Eigen_hyperplane3 near_plane() const;
	Eigen_hyperplane3 far_plane() const;
	Eigen_hyperplane3 left_plane() const;
	Eigen_hyperplane3 right_plane() const;
	Eigen_hyperplane3 bottom_plane() const;
	Eigen_hyperplane3 top_plane() const;
	
	planes_array planes() const;
	corners_array corners() const;
	edges_array edges() const;

	bool contains(const Eigen_vec3&, bool consider_z_planes = true) const;
	intersection contains(const bounding_box&) const;
	static intersection contains(const planes_array&, const bounding_box&);
	
	view_frustum transform(const Eigen_affine3&) const;
};

}

#endif
