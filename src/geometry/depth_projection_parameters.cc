#include "depth_projection_parameters.h"

namespace mf {

bool depth_projection_parameters::valid() const {
	if(z_near < 0.0 || z_far < 0.0) return false;
	if(z_far <= z_near) return false;
	return true;
}

}
