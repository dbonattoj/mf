#ifndef MF_IMAGE_CAMERA_H_
#define MF_IMAGE_CAMERA_H_

#include <cstddef>
#include <array>
#include "camera.h"
#include "../common.h"
#include "../ndarray/ndspan.h"

namespace mf {

/// Base class for camera which handles mapping to image coordinates.
/** Handles mapping between 2D real image coordinates and integer pixel coordinates, and bounds checking.
 ** Does not add scaling or offset, image coordinates map to center point of the square pixel. Pixel coordinates
 ** can be flipped. */
class image_camera {
private:
	ndsize<2> image_size_;
	bool flipped_ = false;
	
protected:
	explicit image_camera(const ndsize<2>&);
	
public:
	using pixel_coordinates_type = ndptrdiff<2>;

	image_camera(const image_camera&) = default;
	
	real image_aspect_ratio() const;
	const ndsize<2>& image_size() const { return image_size_; }
	std::size_t image_number_of_pixels() const { return image_size_.product(); }

	bool pixel_coordinates_flipped() const { return flipped_; }
	void flip_pixel_coordinates();

	
	/// Set image width, and adjust height to keep same aspect ratio.
	void set_image_width(std::size_t);
	
	/// Set image height, and adjust width to keep same aspect ratio.
	void set_image_height(std::size_t);
		
	ndspan<2> image_span() const {
		return ndspan<2>( {0, 0}, {image_size_[0], image_size_[1]} );
	}
		
	camera::image_coordinates to_image(pixel_coordinates_type pix) const {
		auto x = static_cast<real>(pix[0]) + 0.5;
		auto y = static_cast<real>(pix[1]) + 0.5;
		return flipped_ ? {y, x} : {x, y};
	}
	
	pixel_coordinates_type to_pixel(camera::image_coordinates im) const {
		auto x = static_cast<std::ptrdiff_t>(im[0] - 0.5);
		auto y = static_cast<std::ptrdiff_t>(im[1] - 0.5);
		return flipped_ ? {y, x} : {x, y};
	}
};

}

#endif
