#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include ""
#include "depth_camera.h"
#include "../geometry/projection_frustum.h"

namespace mf {

/// Pin-hole camera with projection to a planar image space.
class projection_camera : public depth_camera {
public:
	struct image_parameters {
		Eigen_vec2 scale;
		Eigen_vec2 translation;
	};
	
private:
	Eigen_mat4 projection_matrix_;
	projection_view_frustum frustum_;
	
	static angle angle_between_(const Eigen::Vector3f&, const Eigen::Vector3f&);
	static angle angle_between_(const Eigen::Vector4f&, const Eigen::Vector4f&);

public:		
	projection_camera(const pose&, const projection_view_frustum&, const image_parameters&);
	
	projection_camera(const pose&, const Eigen_mat2x3& intrinsic_matrix, const image_parameters&);


	/// Get projected depth of a 3D point.
	float projected_depth(const Eigen::Vector3f&) const;
	
	/// Project 3D point coordinates to 2D projected coordinates.
	projected_coordinates_type to_projected(const Eigen::Vector3f&) const;
	
	/// Project 3D point coordinates to 2D projected coordinates and get projected depth.
	projected_coordinates_type to_projected(const Eigen::Vector3f&, float& proj_depth) const;
	
	Eigen::Vector3f point(projected_coordinates_type im, float depth) const;
	Eigen::Vector3f point_with_projected_depth(projected_coordinates_type, float proj_depth) const;

};


Eigen::Projective3f homography_transformation(const projection_camera& from, const projection_camera& to);


}

#endif
