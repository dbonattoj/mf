#ifndef MF_CAMERA_IMAGE_H_
#define MF_CAMERA_IMAGE_H_

#include "image.h"
#include "../camera/projection_image_camera.h"
#include <utility>

namespace mf {

template<typename Pixel>
class camera_image : public masked_image<Pixel> {
	using base = masked_image<Pixel>;

private:
	projection_image_camera camera_;

public:
	camera_image(const base&, const projection_camera&);
	camera_image(base&&, const projection_camera&);
	
	const projection_image_camera& this_camera() const { return camera_; }
	projection_image_camera& this_camera() { return camera_; }
};

}

#include "camera_image.tcc"

#endif
