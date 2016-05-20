/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MF_DEPTH_IMAGE_CAMERA_H_
#define MF_DEPTH_IMAGE_CAMERA_H_

#include "image_camera.h"
#include <limits>
#include <type_traits>

namespace mf {

/// Depth camera which handles mapping to integer depth values, base class.
template<typename Depth>
class depth_image_camera : public image_camera {
	static_assert(std::is_integral<Depth>::value, "Depth must be integer type");
	
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
};

}

#include "depth_image_camera.tcc"

#endif
