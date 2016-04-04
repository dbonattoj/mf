#include "frustum.h"
#include "bounding_box.h"

namespace mf {

frustum::frustum(const Eigen_mat4& m) :
	matrix(m) { }


plane frustum::near_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) + m(0,2), m(1,3) + m(1,2), m(2,3) + m(2,2), m(3,3) + m(3,2));
}

plane frustum::far_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) - m(0,2), m(1,3) - m(1,2), m(2,3) - m(2,2), m(3,3) - m(3,2));
}

plane frustum::left_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) + m(0,0), m(1,3) + m(1,0), m(2,3) + m(2,0), m(3,3) + m(3,0));
}

plane frustum::right_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) - m(0,0), m(1,3) - m(1,0), m(2,3) - m(2,0), m(3,3) - m(3,0));
}

plane frustum::bottom_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) + m(0,1), m(1,3) + m(1,1), m(2,3) + m(2,1), m(3,3) + m(3,1));
}

plane frustum::top_plane() const {
	Eigen_mat4 m = view_projection_matrix_.transpose();
	return plane(m(0,3) - m(0,1), m(1,3) - m(1,1), m(2,3) - m(2,1), m(3,3) - m(3,1));
}


frustum::planes_array frustum::planes() const {
	return {
		near_plane(),
		far_plane(),
		left_plane(),
		right_plane(),
		bottom_plane(),
		top_plane()
	};
}


frustum::corners_array frustum::corners() const {
	Eigen_mat4 view_projection_inv = matrix.inverse();
	
	std::array<Eigen_vec3, 8> corn {
		Eigen_vec3(-1, -1, -1),
		Eigen_vec3(-1, +1, -1),
		Eigen_vec3(-1, +1, +1),
		Eigen_vec3(-1, -1, +1),
		Eigen_vec3(+1, -1, -1),
		Eigen_vec3(+1, +1, -1),
		Eigen_vec3(+1, +1, +1),
		Eigen_vec3(+1, -1, +1),
	};
	
	for(Eigen_vec3& p : corn) {
		Eigen_vec4 world_point = view_projection_inv * p.homogeneous();
		p = world_point.hnormalized();
	}
	
	return corn;
}


frustum::edges_array frustum::edges() const {
	auto corn = corners();
	return {
		// Near Plane
		edge(corn[0], corn[1]),
		edge(corn[1], corn[2]),
		edge(corn[2], corn[3]),
		edge(corn[3], corn[0]),
		
		// Far Plane
		edge(corn[0+4], corn[1+4]),
		edge(corn[1+4], corn[2+4]),
		edge(corn[2+4], corn[3+4]),
		edge(corn[3+4], corn[0+4]),
		
		// Connecting
		edge(corn[0], corn[0+4]),
		edge(corn[1], corn[1+4]),
		edge(corn[2], corn[2+4]),
		edge(corn[3], corn[3+4])
	};
}




bool frustum::contains(const Eigen::Vector3f& world_point, bool consider_z_planes) const {
	Eigen_vec4 projected_point = (matrix * world_point.homogeneous()).hnormalized();
	if(projected_point[0] < -1.0 || projected_point[0] > 1.0) return false;
	if(projected_point[1] < -1.0 || projected_point[1] > 1.0) return false;
	if(consider_z_planes && (projected_point[2] < 0.0 || projected_point[2] > 1.0)) return false;
	return true;
}


frustum::intersection frustum::contains(const bounding_box& box) const {
	return contains(planes(), box);
}


frustum::intersection frustum::contains(const planes_array& fr_planes, const bounding_box& box) {
	const Eigen_vec3& a = box.origin;
	const Eigen_vec3& b = box.extremity;
	std::size_t c, c2 = 0;
	for(const plane& p : fr_planes) {
		c = 0;
		if(p.normal[0]*a[0] + p.normal[1]*a[1] + p.normal[2]*a[2] + p.distance > 0) ++c;
		if(p.normal[0]*b[0] + p.normal[1]*a[1] + p.normal[2]*a[2] + p.distance > 0) ++c;
		if(p.normal[0]*a[0] + p.normal[1]*b[1] + p.normal[2]*a[2] + p.distance > 0) ++c;
		if(p.normal[0]*b[0] + p.normal[1]*b[1] + p.normal[2]*a[2] + p.distance > 0) ++c;
		if(p.normal[0]*a[0] + p.normal[1]*a[1] + p.normal[2]*b[2] + p.distance > 0) ++c;
		if(p.normal[0]*b[0] + p.normal[1]*a[1] + p.normal[2]*b[2] + p.distance > 0) ++c;
		if(p.normal[0]*a[0] + p.normal[1]*b[1] + p.normal[2]*b[2] + p.distance > 0) ++c;
		if(p.normal[0]*b[0] + p.normal[1]*b[1] + p.normal[2]*b[2] + p.distance > 0) ++c;
		if(c == 0) return outside_frustum;
		if(c == 8) ++c2;
	}
	return (c2 == 6) ? inside_frustum : partially_inside_frustum;
}


frustum frustum::transform(const Eigen::Affine3f& t) const {
	return frustum(matrix * t.matrix());
}

}
