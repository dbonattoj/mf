#ifndef MF_RANGE_IMAGE_CAMERA_H_
#define MF_RANGE_IMAGE_CAMERA_H_

#include "range_camera.h"
#include "image_camera.h"
#include <array>

namespace mf {

/// Range camera which additionally handles mapping onto pixel grid.
/** Image pixel XY coordinates linearily map to angles, aka projection on sphere.*/
class range_image_camera : public range_camera, public image_camera {
public:
	range_image_camera() = default;
	range_image_camera(const pose&, angle width, angle height, std::size_t imw, std::size_t imh);
	range_image_camera(const pose&, const angle_pair& x_limits, const angle_pair& y_limits, std::size_t imw, std::size_t imh);
	range_image_camera(const range_image_camera&) = default;

	angle angular_resolution_x() const;
	angle angular_resolution_y() const;
	
	pixel_coordinates_type to_image(const Eigen::Vector3f&) const override;
	Eigen::Vector3f point(pixel_coordinates_type, float depth) const override;
};

}

#endif
