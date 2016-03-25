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

	/// Angle of field of view on Y=0 plane.
	virtual angle field_of_view_width() const;
	
	/// Angle of field of view on X=0 plane.
	virtual angle field_of_view_height() const;
	
	/// Positive and negative angles of field of view on Y=0 plane, relative to -Z.
	virtual angle_pair field_of_view_limits_x() const = 0;
	
	/// Positive and negative angles of field of view on X=0 plane, relative to -Z.
	virtual angle_pair field_of_view_limits_y() const = 0;
	
	/// Checks whether 3D point \a p is in camera field of view.
	virtual bool in_field_of_view(const Eigen::Vector3f& p) const = 0;

	/// Extrinsic pose of camera.
	/** Is transformation from parent space object coordinate system to coordinate system of the camera. */
	Eigen::Affine3f view_transformation() const;
		
	/// Direction vector of ray -Z pointing straight out camera.
	/** Relative to parent space object coordinate system. */
	Eigen::Vector3f view_ray_direction() const;

	/// Squared distance of 3D point \a p to camera center.
	/** \a p is in parent space object coordinate system. */
	float depth_sq(const Eigen::Vector3f& p) const;

	/// Distance of 3D point \a p to camera center.
	/** \a p is in parent space object coordinate system. */
	float depth(const Eigen::Vector3f& p) const;	

	/// Convert 3D point cartesian coordinates \a p to spherical.
	/** \a p is in parent space object coordinate system. */
	spherical_coordinates to_spherical(const Eigen::Vector3f& p) const;
	
	/// Convert 3D point spherical coordinates \a s to cartesian.
	/** \a p is in parent space object coordinate system. */	
	Eigen::Vector3f point(const spherical_coordinates& sp) const;
};

}

#endif
