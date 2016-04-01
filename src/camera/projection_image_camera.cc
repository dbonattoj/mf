#include "projection_image_camera.h"
#include "../geometry/plane.h"
#include <stdexcept>
#include "../eigen.h"

namespace mf {

Eigen::Matrix4f projection_image_camera::adjusted_normalized_projection_matrix_
(const Eigen::Matrix4f& mat, std::size_t imw, std::size_t imh) {	
	Eigen::Vector3f sz(imw, imh, 0);
	
	Eigen::Affine3f transformation;
	transformation.scale(sz / 2.0).translate(sz / 2.0);
	
	Eigen::Matrix4f adjusted_mat = transformation * mat;
	return adjusted_mat;
}


projection_image_camera::projection_image_camera
(const pose& ps, const projection_frustum& fr, std::size_t imw, std::size_t imh) :
	projection_camera(ps, adjusted_normalized_projection_matrix_(fr.projection_matrix(), imw, imh)),
	image_camera(imw, imh) { }


projection_image_camera::projection_image_camera
(const pose& ps, const projection_bounding_box& bb, std::size_t imw, std::size_t imh) :
	projection_camera(ps, adjusted_normalized_projection_matrix_(bb.projection_matrix(), imw, imh)),
	image_camera(imw, imh) { }
	
	
projection_image_camera::projection_image_camera
(const projection_camera& cam, bool normalized, std::size_t imw, std::size_t imh) :
	projection_camera(
		cam.relative_pose(),
		normalized ?
			adjusted_normalized_projection_matrix_(cam.projection_matrix(), imw, imh) :
			cam.projection_matrix()
	),
	image_camera(imw, imh) { }


auto projection_image_camera::to_projected(pixel_coordinates_type im) const -> projected_coordinates_type {
	return projected_coordinates_type(
		static_cast<float>(im[0]),
		static_cast<float>(im[1])
	);
}


auto projection_image_camera::to_image(projected_coordinates_type proj) const -> pixel_coordinates_type {
	return pixel_coordinates_type{
		static_cast<std::ptrdiff_t>(proj[0]),
		static_cast<std::ptrdiff_t>(proj[1])
	};
}


auto projection_image_camera::to_image(const Eigen::Vector3f& p) const -> pixel_coordinates_type {
	return to_image(projection_camera::to_projected(p));
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


Eigen::Vector3f projection_image_camera::point_with_projected_depth(pixel_coordinates_type im, float proj_depth) const {
	Eigen_vec2 proj = to_projected(im);
	proj[0] *= proj_depth;
	proj[1] *= proj_depth;
	

	return projection_camera::point_with_projected_depth(proj, proj_depth);
}


Eigen::Vector3f projection_image_camera::point(pixel_coordinates_type im, float depth) const {
	return projection_camera::point(to_projected(im), depth);
}


Eigen::ParametrizedLine<float, 3> projection_image_camera::ray(pixel_coordinates_type im) const {
	return projection_camera::ray(to_projected(im));
}


}
