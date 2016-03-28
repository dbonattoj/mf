#ifndef MF_POINT_CLOUD_POINT_H_
#define MF_POINT_CLOUD_POINT_H_

#include "../color.h"
#include "../eigen.h"

namespace mf {

struct point_xyz {
	Eigen_vec3 coordinates;
};

}

#endif
