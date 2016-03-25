#ifndef MF_PROJECTION_IMAGE_CAMERA_H_
#define MF_PROJECTION_IMAGE_CAMERA_H_

#include "projection_camera.h"
#include "image_camera.h"

namespace mf {

struct plane;

/// Projection camera which additionally handles mapping onto pixel grid.
class projection_image_camera : public projection_camera, public image_camera {
private:
	static Eigen::Matrix4f adjusted_normalized_projection_matrix_
		(const Eigen::Matrix4f&, std::size_t imw, std::size_t imh);

public:
	projection_image_camera() = default;
	
	/// Create perspective projection image camera from frustum and image size.
	/** Underlying projection matrix is set up so that near plane of frustum (i.e. projection plane of the
	 ** `projection_camera` maps to image pixels `[0, width,height[`, and view ray to `(width/2, height/2)`. */
	projection_image_camera(const pose&, const projection_frustum&, std::size_t imw, std::size_t imh);
	
	/// Create orthogonal projection image camera from bounding box and image size.
	/** Underlying projection matrix is set up so that front face of bounding box (i.e. projection plane of the
	 ** `projection_camera` maps to image pixels `[0, width,height[`, and view ray to `(width/2, height/2)`. */
	projection_image_camera(const pose&, const projection_bounding_box&, std::size_t imw, std::size_t imh);
	
	/// Create projection image camera from projection camera and image size.
	/** If `normalized` is true, \a cam is understood to map 3D points in its viewing frustum to coordinates in
	 ** [-1, +1] in the image plane. Then the constructed `projection_image_camera` is adjusted to instead map to
	 ** `[0, width,height[`, centered in `(width/2, height/2)`.
	 ** Otherwise if `normalized` is false then \a cam is left unchanged, and \a imw and \a imh must be compatible
	 ** with the intrinsic parameters of \a cam. */
	projection_image_camera(const projection_camera& cam, bool normalized, std::size_t imw, std::size_t imh);
	
	projection_image_camera(const projection_image_camera&) = default;
	
	Eigen::Vector3f point_with_projected_depth(coordinates_type, float proj_depth) const;
	
	/// Map 2D projected coordinates to image pixel coordinates.
	Eigen::Vector2f to_projected(coordinates_type image_coord) const;
	
	/// Map 2D projected coordinates to image pixel coordinates.
	coordinates_type to_image(Eigen::Vector2f projected_coord) const;
	
	coordinates_type to_image(const Eigen::Vector3f&) const override;
	Eigen::Vector3f point(coordinates_type, float depth) const override;
	
	Eigen::ParametrizedLine<float, 3> ray(coordinates_type) const;

	void adjust_field_of_view_x();
	void adjust_field_of_view_y();
};

}

#endif
