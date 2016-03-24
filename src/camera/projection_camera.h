#ifndef MF_PROJECTION_CAMERA_H_
#define MF_PROJECTION_CAMERA_H_

#include "camera.h"
#include "../geometry/projection_frustum.h"
#include "../geometry/projection_bounding_box.h"

namespace mf {

/// Camera representing projection to a planar image space.
/** Represented using its viewing frustum matrix. Cannot have fields of view larger than 180 deg. */
class projection_camera : public camera {	
protected:
	Eigen::Matrix4f projection_matrix_;

	static angle angle_between_(const Eigen::Vector3f&, const Eigen::Vector3f&);
	static angle angle_between_(const Eigen::Vector4f&, const Eigen::Vector4f&);

public:
	projection_camera() = default;
	projection_camera(const pose&, const projection_frustum&);
	projection_camera(const pose&, const projection_bounding_box&);
	projection_camera(const camera&);

	angle field_of_view_width() const override;
	angle field_of_view_height() const override;
	angle_pair field_of_view_limits_x() const override;
	angle_pair field_of_view_limits_y() const override;
	bool in_field_of_view(const Eigen::Vector3f&) const override;

	bool has_viewing_frustum() const override;
	projection_frustum relative_viewing_frustum() const override;
	void set_relative_viewing_frustum(const projection_frustum&);
	
	Eigen::Projective3f projection_transformation() const override;
	
	bool is_orthogonal() const;
	bool is_perspective() const;

	float projected_depth(const Eigen::Vector3f&) const;
	Eigen::Vector2f to_projected(const Eigen::Vector3f&) const;
	Eigen::Vector2f to_projected(const Eigen::Vector3f&, float& proj_depth) const;
	Eigen::Vector3f point_with_projected_depth(const Eigen::Vector2f&, float z) const;

	Eigen::ParametrizedLine<float, 3> ray(const Eigen::Vector2f&) const;
};

}

#endif
