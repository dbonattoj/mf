#ifndef MF_PROJECTION_FRUSTUM_H_
#define MF_PROJECTION_FRUSTUM_H_

#include "frustum.h"
#include "angle.h"
#include <array>
#include <utility>

namespace mf {

/// Frustum without view transformation.
/** Corresponds to projection matrix only. */
class projection_frustum : public frustum {
private:
	static const float default_z_near_;
	static const float default_z_far_;

protected:
	projection_frustum() = default;

public:
	using distance_pair = std::pair<float, float>;
	using angle_pair = std::pair<angle, angle>;

	static projection_frustum symmetric_perspective
		(float near_width, float near_height, float near_z = default_z_near_, float far_z = default_z_far_);
	static projection_frustum symmetric_perspective_fov
		(angle near_width, angle near_height, float near_z = default_z_near_, float far_z = default_z_far_);
	static projection_frustum symmetric_perspective_fov_x
		(angle near_width, float aspect_ratio, float near_z = default_z_near_, float far_z = default_z_far_);
	static projection_frustum symmetric_perspective_fov_y
		(angle near_height, float aspect_ratio, float near_z = default_z_near_, float far_z = default_z_far_);
	static projection_frustum asymmetric_perspective
		(distance_pair near_x, distance_pair near_y, float near_z = default_z_near_, float far_z = default_z_far_);
	static projection_frustum asymmetric_perspective_fov
		(angle_pair near_x, angle_pair near_y, float near_z = default_z_near_, float far_z = default_z_far_);

	/// Create from projection matrix.
	/** The matrix must be a perspective projection matrix (without view or model transformation). */
	explicit projection_frustum(const Eigen::Matrix4f&);
	
	Eigen::Matrix4f projection_matrix() const { return matrix; }
	
	float aspect_ratio() const;
	bool is_symmetric_x() const;
	bool is_symmetric_y() const;
	bool is_symmetric() const;
	
	float near_z() const;
	float far_z() const;
	
	void adjust_fov_x_to_aspect_ratio(float aspect_ratio);
	void adjust_fov_y_to_aspect_ratio(float aspect_ratio);
};

}

#endif
