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

#ifndef MF_DEPTH_PROJECTION_PARAMETERS_H_
#define MF_DEPTH_PROJECTION_PARAMETERS_H_

#include "../common.h"

namespace mf {

/// Parameters of Z to depth projection.
/** Defines how the Z coordinates in view space are mapped to depth values in image space. For points in the direction
 ** of camera depth is always positive, and increases with distance to camera. When view space coordinate system is
 ** right-handed (camera looks at -Z), `flip_z` needs to be set. */
struct depth_projection_parameters {
	enum depth_range {
		signed_normalized,   ///< Depths of points within frustum are in [-1, +1] (OpenGL convention).
		unsigned_normalized, ///< Depths of points within frustum are in [0, 1] (DirectX convention).
		unsigned_normalized_disparity
	};

	real z_near; ///< Unsigned Z distance of near clipping plane to camera.
	real z_far;	///< Unsigned Z distance of far clipping plane to camera.
	depth_range range = unsigned_normalized; ///< Specifies range of depth values for points in frustum.
	bool flip_z = true; ///< Whether Z axis needs to be reversed, i.e. -Z direction maps to positive depth.
	
	bool valid() const;
	
	real depth_min() const { return (range == signed_normalized ? -1.0 : 0.0); }
	real depth_max() const { return 1.0; }
};

}

#endif
