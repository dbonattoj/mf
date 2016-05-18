#ifndef MF_PROJECTION_IMAGE_CAMERA_H_
#define MF_PROJECTION_IMAGE_CAMERA_H_

#include "projection_camera.h"
#include "depth_image_camera.h"
#include "../ndarray/ndcoord.h"

namespace mf {

/// Pin-hole camera with mapping onto image pixel coordinates.
template<typename Depth>
class projection_image_camera : public projection_camera, public depth_image_camera<Depth> {
	using depth_image_camera_base = depth_image_camera<Depth>;

public:
	projection_image_camera(const projection_camera& cam, ndsize<2> sz);
	projection_image_camera(const projection_image_camera&) = default;
	
	projection_image_camera& operator=(const projection_image_camera&) = default;
};

}

#include "projection_image_camera.tcc"

#endif
