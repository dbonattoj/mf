#ifndef MF_DEPTH_PROJECTION_PARAMETERS_H_
#define MF_DEPTH_PROJECTION_PARAMETERS_H_

namespace mf {

/// Parameters of Z to depth projection.
/** Defines how the Z coordinates in view space are mapped to depth values in image space. For points in the direction
 ** of camera depth is always positive, and increases with distance to camera. When view space coordinate system is
 ** right-handed (camera looks at -Z), `flip_z` needs to be set. */
struct depth_projection_parameters {
	enum depth_range {
		signed_normalized;  ///< Depths of points within frustum are in [-1, +1] (OpenGL convention).
		unsigned_normalized; ///< Depths of points within frustum are in [0, 1] (DirectX convention).
	};

	real z_near = 0.1; ///< Unsigned Z distance of near clipping plane to camera.
	real z_far; ///< Unsigned Z distance of far clipping plane to camera.
	depth_range range = unsigned_normalized; ///< Specifies range of depth values for points in frustum.
	bool flip_z = true; ///< Whether Z axis needs to be reversed, i.e. -Z direction maps to positive depth.
	
	bool valid() const;
	
	real depth_min() const { return (range == unsigned_normalized ? 0.0 : -1.0); }
	real depth_max() const { return 1.0; }
};

}

#endif
