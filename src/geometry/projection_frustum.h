#ifndef MF_PROJECTION_VIEW_FRUSTUM_H_
#define MF_PROJECTION_VIEW_FRUSTUM_H_

#include "view_frustum.h"
#include "angle.h"
#include "../eigen.h"
#include <utility>
#include <array>

namespace mf {


/// View frustum of a perspective camera, without pose..
class projection_view_frustum : public view_frustum {
private:
	projection_view_frustum(const Eigen_mat4& mat, const projection_depth_parameters& conv) :
		view_frustum(mat, conv) { }

	static std::array<real, 2> depth_components_(const projection_depth_parameters&);

public:
	using distance_pair = std::pair<real, real>;
	using angle_pair = std::pair<angle, angle>;

	static projection_view_frustum symmetric_perspective
		(real near_width, real near_height, const projection_depth_parameters&);
		
	static projection_view_frustum symmetric_perspective_fov
		(angle near_width, angle near_height, const projection_depth_parameters&);
		
	static projection_view_frustum symmetric_perspective_fov_x
		(angle near_width, real aspect_ratio, const projection_depth_parameters&);
		
	static projection_view_frustum symmetric_perspective_fov_y
		(angle near_height, real aspect_ratio, const projection_depth_parameters&);
		
	static projection_view_frustum asymmetric_perspective
		(distance_pair near_x, distance_pair near_y, const projection_depth_parameters&);
		
	static projection_view_frustum asymmetric_perspective_fov
		(angle_pair near_x, angle_pair near_y, const projection_depth_parameters&);
	
	Eigen_mat4 projection_matrix() const { return frustum::view_projection_matrix(); }
	
	real aspect_ratio() const;
	bool is_symmetric_x() const;
	bool is_symmetric_y() const;
	bool is_symmetric() const;
	
	real near_z() const;
	real far_z() const;
	
	void adjust_fov_x_to_aspect_ratio(real aspect_ratio);
	void adjust_fov_y_to_aspect_ratio(real aspect_ratio);
};

}

#endif
