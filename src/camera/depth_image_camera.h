#ifndef MF_DEPTH_IMAGE_CAMERA_H_
#define MF_DEPTH_IMAGE_CAMERA_H_

#include "image_camera.h"
#include <limits>
#include <type_traits>

namespace mf {

/// Base class for depth camera which handles mapping to integer depth values.
template<typename Depth>
class depth_image_camera : public image_camera {
	static_assert(std::is_integral<Depth>::value, "Depth must be integer type")
	
private:
	real depth_origin_ = 0.0;
	real depth_range_ = +1.0;
	real pixel_depth_minimum_ = static_cast<real>(std::numeric_limits<Depth>::min());
	real pixel_depth_maximum_ = static_cast<real>(std::numeric_limits<Depth>::max());

public:
	using pixel_depth_type = Depth; ///< Pixel depth value integer type.

	depth_image_camera() = default;
	depth_image_camera(const ndsize<2>& image_size, real origin, real range) :
		image_camera(image_size), depth_origin_(origin), depth_range_(range) { }
	
	/// Verifies whether \a d is in bounds of valid depth values.
	bool in_depth_bounds(real d) const;
	
	/// Map depth value to pixel depth value.
	/** \a d must be in bounds, i.e. `in_depth_bounds(d)` must be true. */
	pixel_depth_type to_pixel_depth(real d) const;
	
	/// Map depth value to pixel depth value, and clamp to bounds.
	/** If \a d is not in bounds, result is clamped between minimum and maximum pixel depth. */
	pixel_depth_type to_pixel_depth_clamp(real d) const;
	
	/// Map pixel depth value to depth value in range defined by depth projection parameter.
	/** Values outside pixel depth range are mapped to values outside real depth range. */
	real to_depth(pixel_depth_type pixd) const;
	
	/// Verify whether pixel depth decreases as depth increases.
	bool pixel_depth_flipped() const;
	
	/// Flip pixel depths range.
	void flip_pixel_depth();
};

}

#include "depth_image_camera.tcc"

#endif
