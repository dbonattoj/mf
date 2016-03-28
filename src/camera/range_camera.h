#ifndef MF_RANGE_CAMERA_H_
#define MF_RANGE_CAMERA_H_

#include "camera.h"
#include "../geometry/angle.h"
#include <utility>

namespace mf {

/// Camera representing projection onto sphere.
class range_camera : public camera {
protected:
	angle_pair azimuth_limits_;
	angle_pair elevation_limits_;
	
public:
	range_camera() = default;
	range_camera(const pose&, angle width, angle height);
	range_camera(const pose&, const angle_pair& x_limits, const angle_pair& y_limits);

	angle field_of_view_width() const override;
	angle field_of_view_height() const override;
	angle_pair field_of_view_limits_x() const override;
	angle_pair field_of_view_limits_y() const override;
	bool in_field_of_view(const Eigen::Vector3f&) const override;
};

}

#endif
