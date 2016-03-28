#ifndef MF_IMAGE_CAMERA_H_
#define MF_IMAGE_CAMERA_H_

#include <cstddef>
#include <array>
#include "../eigen.h"
#include "../ndarray/ndcoord.h"

namespace mf {

/// Abstract (secondary) base class for camera which handles mapping to image coordinates.
/** Image pixels may be graduation of projection plane (projection), or of spherical coordinates (range).
 ** Handles mapping of 2D projected coordinates to integer pixel coordinates, and bounds checking.  */
class image_camera {
protected:
	std::size_t image_width_;
	std::size_t image_height_;
	
	image_camera() = default;
	image_camera(std::size_t imw, std::size_t imh);
	
public:
	using pixel_coordinates_type = ndptrdiff<2>;

	image_camera(const image_camera&) = default;
	
	float image_aspect_ratio() const;
	ndsize<2> image_size() const { return make_ndsize(image_width_, image_height_); }
	std::size_t image_number_of_pixels() const;
	void set_image_size(ndsize<2>);
	
	/// Set image width, and adjust height to keep same aspect ratio.
	void set_image_width(std::size_t);
	
	/// Set image height, and adjust width to keep same aspect ratio.
	void set_image_height(std::size_t);
	
	/// Check whether given coordinates are in image bounds.
	bool in_bounds(pixel_coordinates_type) const;
	
	/// Map 3D point in space to image coordinates.
	virtual pixel_coordinates_type to_image(const Eigen::Vector3f&) const = 0;
	
	/// Inverse mapping of image coordinates to point in space.
	/** Depth is the distance of the point to the camera origin. `projection_image_camera` also provides a version
	 ** which takes the projected depth instead. */
	virtual Eigen::Vector3f point(pixel_coordinates_type, float depth) const = 0;
};

}

#endif
