#ifndef MF_POINT_CLOUD_POINT_H_
#define MF_POINT_CLOUD_POINT_H_

#include "../color.h"
#include "../eigen.h"
#include "../elem.h"
#include "../elem_tuple.h"

namespace mf {

/// Point cloud point with only XYZ coordinates.
/** Can be marked valid and invalid. Stored using 4-vector with homogeneous coordinates, where last component is fixed
 ** to 1.0: allows for SIMD processing. When marked invalid last component is set to non 1.0. */
struct point_xyz {
	Eigen_vec4 homogeneous_coordinates;
	
	point_xyz() :
		homogeneous_coordinates(Eigen_vec4::Zero()) { }
	
	point_xyz(const Eigen_vec3& pos) :
		homogeneous_coordinates(pos[0], pos[1], pos[2], 1.0) { }
	
	auto position() { return homogeneous_coordinates.head<3>(); }
	auto position() const { return homogeneous_coordinates.head<3>(); }
	
	Eigen_scalar& operator[](std::ptrdiff_t i) { return homogeneous_coordinates[i]; }
	const Eigen_scalar& operator[](std::ptrdiff_t i) const { return homogeneous_coordinates[i]; }
	
	explicit operator bool () const { return homogeneous_coordinates[3] == 1.0; }
	void validate() { homogeneous_coordinates[3] = 1.0; }
	void invalidate() { homogeneous_coordinates[3] = 0.0; }
};


template<>
struct elem_traits<point_xyz> {
	using scalar_type = Eigen_scalar;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = 3;
	constexpr static std::size_t stride = sizeof(Eigen_scalar);
	constexpr static std::size_t offset = 0;
};


/// Point cloud point with XYZ coordinates, and RGB color.
struct point_xyzrgb : elem_tuple<point_xyz, rgb_color> {
	using base = elem_tuple<point_xyz, rgb_color>;

	point_xyzrgb(const base& tup) : base(tup) { }
	
	point_xyzrgb(const point_xyz& xyz) :
		base(Eigen_vec3(xyz.position()), rgb_color{255,255,255}) { }
		
	auto position() { return get<0>(*this).position(); }
	auto position() const { return get<0>(*this).position(); }
	
	rgb_color& color() { return get<1>(*this); }
	const rgb_color& color() const { return get<1>(*this); }

	Eigen_scalar& operator[](std::ptrdiff_t i) { return get<0>(*this).homogeneous_coordinates[i]; }
	const Eigen_scalar& operator[](std::ptrdiff_t i) const { return get<0>(*this).homogeneous_coordinates[i]; }
};

/// Point cloud point with XYZ coordinates, normal vector, weight, and RGB color.
struct point_full : elem_tuple<point_xyz, Eigen_vec3, Eigen_scalar, rgb_color> {
	using base = elem_tuple<point_xyz, Eigen_vec3, Eigen_scalar, rgb_color>;

	point_full(const base& tup) : base(tup) { }

	point_full(const point_xyz& xyz) : 
		base(Eigen_vec3(xyz.position()), Eigen_vec3::Zero(), 1.0, rgb_color{255,255,255}) { }

	point_full(const point_xyzrgb& xyzrgb) : 
		base(Eigen_vec3(xyzrgb.position()), Eigen_vec3::Zero(), 1.0, xyzrgb.color()) { }

	auto position() { return get<0>(*this).position(); }
	auto position() const { return get<0>(*this).position(); }
	
	Eigen_vec3& normal() { return get<1>(*this); }
	const Eigen_vec3& normal() const { return get<1>(*this); }

	Eigen_scalar& weight() { return get<2>(*this); }
	const Eigen_scalar& weight() const { return get<2>(*this); }
	
	rgb_color& color() { return get<3>(*this); }
	const rgb_color& color() const { return get<3>(*this); }

	Eigen_scalar& operator[](std::ptrdiff_t i) { return get<0>(*this).homogeneous_coordinates[i]; }
	const Eigen_scalar& operator[](std::ptrdiff_t i) const { return get<0>(*this).homogeneous_coordinates[i]; }
};


}

#endif
