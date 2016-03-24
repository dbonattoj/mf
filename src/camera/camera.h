#ifndef MF_CAMERA_H_
#define MF_CAMERA_H_

#include "../eigen.h"
#include <array>
#include "../space_object.h"
#include "../geometry/angle.h"
#include "../geometry/pose.h"
#include "../geometry/spherical_coordinates.h"
#include "../geometry/projection_frustum.h"

namespace mf {

/// Abstract base class for camera.
class camera : public space_object {
protected:
	camera() = default;
	camera(const camera&) = default;
	explicit camera(const pose&);

public:
	using angle_pair = std::pair<angle, angle>;

	virtual ~camera() = default;

	virtual angle field_of_view_width() const;
	virtual angle field_of_view_height() const;
	virtual angle_pair field_of_view_limits_x() const = 0;
	virtual angle_pair field_of_view_limits_y() const = 0;
	virtual bool in_field_of_view(const Eigen::Vector3f&) const = 0;

	virtual bool has_viewing_frustum() const = 0;
	virtual projection_frustum relative_viewing_frustum() const = 0;	
	frustum viewing_frustum() const;

	virtual Eigen::Projective3f projection_transformation() const = 0;
	Eigen::Affine3f view_transformation() const;
	Eigen::Projective3f view_projection_transformation() const;
			
	Eigen::Vector3f view_ray_direction() const;

	float depth_sq(const Eigen::Vector3f&) const;
	float depth(const Eigen::Vector3f&) const;	

	spherical_coordinates to_spherical(const Eigen::Vector3f&) const;
	Eigen::Vector3f point(const spherical_coordinates&) const;
};

}

#endif
