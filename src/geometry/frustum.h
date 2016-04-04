#ifndef MF_FRUSTUM_H_
#define MF_FRUSTUM_H_

#include "plane.h"
#include "angle.h"
#include "pose.h"
#include "../eigen.h"
#include <array>
#include <utility>

namespace mf {

struct bounding_box;

/// Pyramid frustum, clipped by near and far planes.
/** Represented using camera view projection matrix. Encodes both intrinsic (shape of frustum) and extrinsic (pose)
 ** parameters. Provides extraction of the 6 planes, the corners and edges, and intersection test with bounding box. */
class frustum {
private:
	Eigen_mat4 view_projection_matrix_;

public:
	enum intersection {
		outside_frustum = 0,
		inside_frustum,
		partially_inside_frustum
	};
	
	using edge = std::pair<Eigen_vec3, Eigen_vec3>;
	using corners_array = std::array<Eigen_vec3, 8>;
	using edges_array = std::array<edge, 12>;
	using planes_array = std::array<plane, 6>;
	
	frustum() = default;
	frustum(const frustum&) = default;
	explicit frustum(const Eigen_mat4& mvp);
	
	const Eigen_mat4& view_projection_matrix() const { return view_projection_matrix_; }

	// TODO replace plane by Eigen_hyperplane3

	plane near_plane() const;
	plane far_plane() const;
	plane left_plane() const;
	plane right_plane() const;
	plane bottom_plane() const;
	plane top_plane() const;
	
	planes_array planes() const;
	corners_array corners() const;
	edges_array edges() const;
								
	bool contains(const Eigen_vec3&, bool consider_z_planes = true) const;
	intersection contains(const bounding_box&) const;
	static intersection contains(const planes_array&, const bounding_box&);
	
	frustum transform(const Eigen_affine3&) const;
};

}

#endif
