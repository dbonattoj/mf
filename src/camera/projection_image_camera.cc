#include "projection_image_camera.h"
#include "../geometry/plane.h"
#include <stdexcept>
#include "../eigen.h"

namespace mf {

Eigen::Matrix4f projection_image_camera::adjusted_normalized_projection_matrix_
(const Eigen::Matrix4f& mat, std::size_t imw, std::size_t imh) {	
	float w = static_cast<float>(imw), h = static_cast<float>(imh);
	
	Eigen::Affine3f transformation;
	transformation.scale(w, h).translate(w/2.0f, h/2.0f);
	
	Eigen::Matrix4f adjusted_mat = transformation * mat;
	return adjusted_mat;
}


projection_image_camera::projection_image_camera(const pose& ps, const projection_frustum& fr, std::size_t imw, std::size_t imh) :
	projection_camera(ps, adjusted_normalized_projection_matrix_(fr.projection_matrix(), imw, imh)),
	image_camera(imw, imh) { }


projection_image_camera::projection_image_camera(const pose& ps, const projection_bounding_box& bb, std::size_t imw, std::size_t imh) :
	projection_camera(ps, adjusted_normalized_projection_matrix_(bb.projection_matrix(), imw, imh)),
	image_camera(imw, imh) { }
	
	

projection_image_camera::projection_image_camera(const projection_camera& cam, bool normalized, std::size_t imw, std::size_t imh) :
	projection_camera(
		cam.relative_pose(),
		normalized ?
			cam.projection_matrix() :
			adjusted_normalized_projection_matrix_(cam.projection_matrix(), imw, imh)		
	),
	image_camera(imw, imh) { }


Eigen::Vector2f projection_camera::to_projected(coordinates_type image_coord) const {
	return Eigen::Vector2f(
		static_cast<float>(image_coord[0]),
		static_cast<float>(image_coord[1])
	);
}


coordinates_type projection_camera::to_image(Eigen::Vector2f projected_coord) const {
	return coordinates_type(
		static_cast<std::ptrdiff_t>(projected_coord[0]),
		static_cast<std::ptrdiff_t>(projected_coord[1])
	);
}


auto projection_image_camera::to_image(const Eigen::Vector3f& p) const -> coordinates_type {
	Eigen::Vector2f proj = to_projected(p);
	std::ptrdiff_t x = static_cast<std::ptrdiff_t>(proj[0]);
	std::ptrdiff_t y = static_cast<std::ptrdiff_t>(proj[1]);
	return {x, y};
}


void projection_image_camera::adjust_field_of_view_x() {
	projection_frustum fr = relative_viewing_frustum();
	fr.adjust_fov_x_to_aspect_ratio( image_aspect_ratio() );
	set_relative_viewing_frustum(fr);
}


void projection_image_camera::adjust_field_of_view_y() {		
	projection_frustum fr = relative_viewing_frustum();
	fr.adjust_fov_y_to_aspect_ratio( image_aspect_ratio() );
	set_relative_viewing_frustum(fr);
}


Eigen::Vector3f projection_image_camera::point_with_projected_depth(coordinates_type im, float proj_depth) const {
	Eigen::Vector2f proj;
	proj[0] = static_cast<float>(im[0]);
	proj[1] = static_cast<float>(im[1]);
	return projection_camera::point_with_projected_depth(proj, proj_depth);
}

Eigen::Vector3f projection_image_camera::point(coordinates_type im, float depth) const {
	const float intermediary_projected_depth = 1.0;
	Eigen::Vector3f p = point_with_projected_depth(im, intermediary_projected_depth);
	spherical_coordinates sp = spherical_coordinates::from_cartesian(p);
	sp.radius = depth;
	return sp.to_cartesian();
}


Eigen::ParametrizedLine<float, 3> projection_image_camera::ray(coordinates_type im) const {
	Eigen::Vector2f proj;
	proj[0] = static_cast<float>(im[0]);
	proj[1] = static_cast<float>(im[1]);
	return projection_camera::ray(proj);
}


}
