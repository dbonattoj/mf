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
/** Represented using the view projection matrix. Encodes both intrinsic (shape of frustum) and extrinsic (pose)
 ** parameters. Provides extraction of the 6 planes, the corners and edges, and intersection test with bounding box. */
struct frustum {
	enum intersection {
		outside_frustum = 0,
		inside_frustum,
		partially_inside_frustum
	};
	
	using edge = std::pair<Eigen::Vector3f, Eigen::Vector3f>;
	using corners_array = std::array<Eigen::Vector3f, 8>;
	using edges_array = std::array<edge, 12>;
	using planes_array = std::array<plane, 6>;
	
	Eigen::Matrix4f matrix;

	frustum() = default;
	frustum(const frustum&) = default;
	explicit frustum(const Eigen::Matrix4f& mvp);

	plane near_plane() const;
	plane far_plane() const;
	plane left_plane() const;
	plane right_plane() const;
	plane bottom_plane() const;
	plane top_plane() const;
	
	planes_array planes() const;
	corners_array corners() const;
	edges_array edges() const;
				
	float projected_depth(const Eigen::Vector3f&, bool clip = true) const;
				
	bool contains(const Eigen::Vector3f&, bool consider_z_planes = true) const;
	intersection contains(const bounding_box&) const;
	static intersection contains(const planes_array&, const bounding_box&);
	
	frustum transform(const Eigen::Affine3f&) const;
};

}

#endif
