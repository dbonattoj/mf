#include "view_frustum.h"
#include <stdexcept>

namespace mf {
	
view_frustum::view_frustum(const Eigen_mat4& mat, const depth_projection_parameters& dparam) :
	view_projection_transformation_(mat),
	depth_parameters_(dparam)
{
	if(! dparam.valid()) throw std::invalid_argument("depth projection parameters not valid");
}


Eigen_hyperplane3 view_frustum::near_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(2, 0);
	real b = m(2, 1);
	real c = m(2, 2);
	real d = m(2, 3);
	if(depth_parameters_.range == depth_projection_parameters::signed_normalized) {
		a += m(3, 0);
		b += m(3, 1);
		c += m(3, 2);
		d += m(3, 3);
	}
	// TODO orientation of plane
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


Eigen_hyperplane3 view_frustum::far_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(3, 0) - m(2, 0);
	real b = m(3, 1) - m(2, 1);
	real c = m(3, 2) - m(2, 2);
	real d = m(3, 3) - m(2, 3);
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


Eigen_hyperplane3 view_frustum::left_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(3, 0) + m(0, 0);
	real b = m(3, 1) + m(0, 1);
	real c = m(3, 2) + m(0, 2);
	real d = m(3, 3) + m(0, 3);
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


Eigen_hyperplane3 view_frustum::right_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(3, 0) - m(0, 0);
	real b = m(3, 1) - m(0, 1);
	real c = m(3, 2) - m(0, 2);
	real d = m(3, 3) - m(0, 3);
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


Eigen_hyperplane3 view_frustum::bottom_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(3, 0) + m(1, 0);
	real b = m(3, 1) + m(1, 1);
	real c = m(3, 2) + m(1, 2);
	real d = m(3, 3) + m(1, 3);
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


Eigen_hyperplane3 view_frustum::top_plane() const {
	const Eigen_mat4& m = matrix_();
	real a = m(3, 0) - m(1, 0);
	real b = m(3, 1) - m(1, 1);
	real c = m(3, 2) - m(1, 2);
	real d = m(3, 3) - m(1, 3);
	return Eigen_hyperplane3(Eigen_vec3(a, b, c), d);
}


auto view_frustum::planes() const -> planes_array {
	return {
		near_plane(),
		far_plane(),
		left_plane(),
		right_plane(),
		bottom_plane(),
		top_plane()
	};
}


auto view_frustum::corners() const -> corners_array {
	Eigen_projective3 view_projection_inv = view_projection_transformation_.inverse();
	
	real n = (depth_parameters_.range == depth_projection_parameters::signed_normalized ? -1.0 : 0.0);
	
	std::array<Eigen_vec3, 8> corn {
		Eigen_vec3(-1.0, -1.0, n),
		Eigen_vec3(-1.0, +1.0, n),
		Eigen_vec3(-1.0, +1.0, 1.0),
		Eigen_vec3(-1.0, -1.0, 1.0),
		Eigen_vec3(+1.0, -1.0, n),
		Eigen_vec3(+1.0, +1.0, n),
		Eigen_vec3(+1.0, +1.0, 1.0),
		Eigen_vec3(+1.0, -1.0, 1.0),
	};
	
	for(Eigen_vec3& p : corn)
		p = (view_projection_inv * p.homogeneous()).eval().hnormalized();
	
	return corn;

}


auto view_frustum::edges() const -> edges_array {
	auto corn = corners();
	return {
		// near Plane
		edge(corn[0], corn[1]),
		edge(corn[1], corn[2]),
		edge(corn[2], corn[3]),
		edge(corn[3], corn[0]),
		
		// far Plane
		edge(corn[0+4], corn[1+4]),
		edge(corn[1+4], corn[2+4]),
		edge(corn[2+4], corn[3+4]),
		edge(corn[3+4], corn[0+4]),
		
		// connecting
		edge(corn[0], corn[0+4]),
		edge(corn[1], corn[1+4]),
		edge(corn[2], corn[2+4]),
		edge(corn[3], corn[3+4])
	};
}


bool view_frustum::contains(const Eigen_vec3& world_point, bool consider_z_planes) const {
	Eigen_vec3 projected_point = (view_projection_transformation_ * world_point.homogeneous()).eval().hnormalized();
	if(projected_point[0] < -1.0 || projected_point[0] > 1.0) return false;
	if(projected_point[1] < -1.0 || projected_point[1] > 1.0) return false;
	if(consider_z_planes && (projected_point[2] < 0.0 || projected_point[2] > 1.0)) return false;
	return true;
}


auto view_frustum::contains(const bounding_box& box) const -> intersection {
	return contains(planes(), box);
}


auto view_frustum::contains(const planes_array& fr_planes, const bounding_box& box) -> intersection {
	const Eigen_vec3& a = box.origin;
	const Eigen_vec3& b = box.extremity;
	int c, c2 = 0;
	for(const Eigen_hyperplane3& p : fr_planes) {
		c = 0;
		if(p.normal()[0]*a[0] + p.normal()[1]*a[1] + p.normal()[2]*a[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*b[0] + p.normal()[1]*a[1] + p.normal()[2]*a[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*a[0] + p.normal()[1]*b[1] + p.normal()[2]*a[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*b[0] + p.normal()[1]*b[1] + p.normal()[2]*a[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*a[0] + p.normal()[1]*a[1] + p.normal()[2]*b[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*b[0] + p.normal()[1]*a[1] + p.normal()[2]*b[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*a[0] + p.normal()[1]*b[1] + p.normal()[2]*b[2] + p.offset() > 0) ++c;
		if(p.normal()[0]*b[0] + p.normal()[1]*b[1] + p.normal()[2]*b[2] + p.offset() > 0) ++c;
		if(c == 0) return outside_frustum;
		if(c == 8) ++c2;
	}
	return (c2 == 6) ? inside_frustum : partially_inside_frustum;
}


view_frustum view_frustum::transform(const Eigen_affine3& trans) const{
	return view_frustum((view_projection_transformation_ * trans).matrix(), depth_parameters_);
}


}
