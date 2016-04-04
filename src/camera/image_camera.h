#ifndef MF_IMAGE_CAMERA_H_
#define MF_IMAGE_CAMERA_H_

#include <cstddef>
#include <array>
#include "../common.h"
#include "../ndarray/ndspan.h"

namespace mf {

/// Camera which handles mapping to image coordinates.
/** Handles mapping between 2D real image coordinates and integer pixel coordinates, and bounds checking. */
class image_camera {
protected:
	ndsize<2> image_size_;
	
	image_camera() = default;
	explicit image_camera(const ndsize<2>&);
	
public:
	using pixel_coordinates_type = ndptrdiff<2>;

	image_camera(const image_camera&) = default;
	
	real image_aspect_ratio() const;
	const ndsize<2>& image_size() const { return image_size_; }
	std::size_t image_number_of_pixels() const { return image_size_.product(); }

	void set_image_size(const ndsize<2>& sz) { image_size_ = sz; }
	
	/// Set image width, and adjust height to keep same aspect ratio.
	void set_image_width(std::size_t);
	
	/// Set image height, and adjust width to keep same aspect ratio.
	void set_image_height(std::size_t);
		
	ndspan<2> image_span() const {
		return ndspan<2>( {0, 0}, {image_size_[0], image_size_[1]} );
	}
};

}

#endif
