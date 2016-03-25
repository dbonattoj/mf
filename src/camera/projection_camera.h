#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include "camera.h"
#include "../geometry/projection_frustum.h"
#include "../geometry/projection_bounding_box.h"

namespace mf {

/// Pin-hole camera with projection to a planar image space.
/** Represented using its 4x4 projection matrix. Cannot have fields of view larger than 180 degrees. */
class projection_camera : public camera {	
protected:
	/// Intrinsic projection parameter of camera.
	/** 4x4 homogeneous matrix which projects 3D points in camera's coordinate system to 2D points on image plane.
	 ** May represent perspective or orthogonal projection. May also include image scaling and offset for mapping onto
	 ** pixel grid of `image_camera`. */
	Eigen::Matrix4f projection_matrix_;

	static angle angle_between_(const Eigen::Vector3f&, const Eigen::Vector3f&);
	static angle angle_between_(const Eigen::Vector4f&, const Eigen::Vector4f&);

public:
	/// 2D coordinates on the planar image plane.
	using projected_coordinates_type = Eigen::Vector2f;

	projection_camera() = default;
	
	/// Create projection camera from pose (extrinsic), and 4x4 intrinsic parameter matrix.
	projection_camera(const pose&, const Eigen::Matrix4f& intrinsic);
	
	/// Create perspective projection camera from pose (extrinsic), and projection frustum.
	/** Positions in frustum are mapped to projected coordinates in [-1, +1]. */
	projection_camera(const pose&, const projection_frustum&);
	
	/// Create orthogonal projection camera from pose (extrinsic), and projection bounding box.
	/** Positions in bounding box are mapped to projected coordinates in [-1, +1]. */
	projection_camera(const pose&, const projection_bounding_box&);

	angle field_of_view_width() const override;
	angle field_of_view_height() const override;
	angle_pair field_of_view_limits_x() const override;
	angle_pair field_of_view_limits_y() const override;
	bool in_field_of_view(const Eigen::Vector3f&) const override;

	projection_frustum relative_viewing_frustum() const;
	void set_relative_viewing_frustum(const projection_frustum&);
	
	Eigen::Projective3f view_projection_transformation() const;
	Eigen::Projective3f projection_transformation() const;
	
	const Eigen::Matrix4f& projection_matrix() const { return projection_matrix_; }
	
	bool is_orthogonal() const;
	bool is_perspective() const;

	float projected_depth(const Eigen::Vector3f&) const;
	projected_coordinates_type to_projected(const Eigen::Vector3f&) const;
	projected_coordinates_type to_projected(const Eigen::Vector3f&, float& proj_depth) const;
	Eigen::Vector3f point_with_projected_depth(projected_coordinates_type, float z) const;

	Eigen::ParametrizedLine<float, 3> ray(const Eigen::Vector2f&) const;
};

}

#endif
