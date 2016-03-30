#ifndef MF_POINT_CLOUD_POINT_H_
#define MF_POINT_CLOUD_POINT_H_

#include "../color.h"
#include "../eigen.h"
#include "../elem.h"
#include "../elem_tuple.h"

namespace mf {

struct point_xyz {
	Eigen_vec4 homogeneous_coordinates;
	
	explicit operator bool () const { return homogeneous_coordinates[3] == 1.0; }
	void validate() { homogeneous_coordinates[3] = 1.0; }
	void invalidate() { homogeneous_coordinates[3] = 0.0; }
};


using point_xyzrgb = elem_tuple<point_xyz, rgb_color>;


template<>
struct elem_traits<point_xyz> {
	using scalar_type = Eigen_scalar;
	constexpr static bool is_tuple = false;
	constexpr static std::size_t components = 3;
	constexpr static std::size_t size = sizeof(point_xyz);
	constexpr static std::size_t stride = sizeof(point_xyz);
};


}

#endif
