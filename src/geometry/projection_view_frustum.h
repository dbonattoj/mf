#ifndef MF_PROJECTION_VIEW_FRUSTUM_H_
#define MF_PROJECTION_VIEW_FRUSTUM_H_

#include "view_frustum.h"
#include "angle.h"
#include "../eigen.h"
#include <utility>
#include <array>

namespace mf {


/// View frustum of a perspective camera, without pose.
/** Frustum apex is at origin, axis-aligned, and pointing in -Z or +Z direction depending on depth projection parameter.
 ** Can be creates for perspective projection. */
class projection_view_frustum : public view_frustum {
private:
	projection_view_frustum(const Eigen_mat4& mat, const depth_projection_parameters& conv) :
		view_frustum(mat, conv) { }

	static std::array<real, 3> depth_components_(const depth_projection_parameters&);

public:
	using distance_pair = std::pair<real, real>;
	using angle_pair = std::pair<angle, angle>;
	
	/// Create with symmetric perspective, given width and height of near clipping plane.
	static projection_view_frustum symmetric_perspective
		(real near_width, real near_height, const depth_projection_parameters&);
		
	/// Create with symmetric perspective, given horizontal and vertical field of view angles.
	static projection_view_frustum symmetric_perspective_fov
		(angle near_width, angle near_height, const depth_projection_parameters&);
	
	/// Create with symmetric perspective, given horizontal field of view and image aspect ratio.
	static projection_view_frustum symmetric_perspective_fov_x
		(angle near_width, real aspect_ratio, const depth_projection_parameters&);

	/// Create with symmetric perspective, given vertical field of view and image aspect ratio.		
	static projection_view_frustum symmetric_perspective_fov_y
		(angle near_height, real aspect_ratio, const depth_projection_parameters&);
	
	/// Create with asymmetric perspective, given horizontal and vertical ranges of near clipping plane.		
	static projection_view_frustum asymmetric_perspective
		(distance_pair near_x, distance_pair near_y, const depth_projection_parameters&);

	/// Create with asymmetric perspective, given horizontal and vertical limit angles of field of view.		
	static projection_view_frustum asymmetric_perspective_fov
		(angle_pair near_x, angle_pair near_y, const depth_projection_parameters&);	
	
	const Eigen_mat4& projection_matrix() const { return frustum::view_projection_matrix(); }
	
	real aspect_ratio() const;
	bool is_symmetric_x() const;
	bool is_symmetric_y() const;
	bool is_symmetric() const;
	
	void adjust_fov_x_to_aspect_ratio(real aspect_ratio);
	void adjust_fov_y_to_aspect_ratio(real aspect_ratio);
};
// TODO add orthogonal projection if needed

}

#endif
