#ifndef MF_DEPTH_CAMERA_H_
#define MF_DEPTH_CAMERA_H_

#include "camera.h"

namespace mf {

/// Camera which maps 3D coordinates of point to depth value in addition to the 2D image coordinates.
class depth_camera : public camera {
public:
	virtual real depth(const Eigen_vec3& p) const = 0;
	virtual real depth(const spherical_coordinates& sp) const {
		return this->depth(camera::point(sp));
	}

	virtual Eigen_vec3 point(const image_coordinates& c, real depth) const = 0;
};
	
}

#endif
