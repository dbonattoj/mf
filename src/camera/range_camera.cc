#include "range_camera.h"
#include "../geometry/math_constants.h"

namespace mf {

range_camera::range_camera(const pose& ps, angle width, angle height) :
	camera(ps),
	azimuth_limits_(-width/2.0f, +width/2.0f),
	elevation_limits_(-height/2.0f, +height/2.0f) { }	


range_camera::range_camera(const pose& ps, const angle_pair& x_limits, const angle_pair& y_limits) :
	camera(ps),
	azimuth_limits_(x_limits),
	elevation_limits_(y_limits) { }


angle range_camera::field_of_view_width() const {
	return azimuth_limits_.second - azimuth_limits_.first;
}


angle range_camera::field_of_view_height() const {
	return elevation_limits_.second - elevation_limits_.first;
}


range_camera::angle_pair range_camera::field_of_view_limits_x() const {
	return azimuth_limits_;
}


range_camera::angle_pair range_camera::field_of_view_limits_y() const {
	return elevation_limits_;
}


bool range_camera::in_field_of_view(const Eigen::Vector3f& p) const {
	spherical_coordinates s = to_spherical(p);
	return (s.azimuth >= azimuth_limits_.first) && (s.azimuth <= azimuth_limits_.second)
	    && (s.elevation >= elevation_limits_.first) && (s.elevation <= elevation_limits_.second);
}


bool range_camera::has_viewing_frustum() const {
	return (field_of_view_width() < pi) && (field_of_view_height() < pi);
}


projection_frustum range_camera::relative_viewing_frustum() const {
	return projection_frustum::asymmetric_perspective_fov(
		field_of_view_limits_x(),
		field_of_view_limits_y()
	);
}


Eigen::Projective3f range_camera::projection_transformation() const {
	return Eigen::Projective3f(relative_viewing_frustum().matrix);
}


}
