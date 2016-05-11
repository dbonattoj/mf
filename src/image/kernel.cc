#include "kernel.h"
#include <algorithm>

namespace mf {

bool_image_kernel disk_image_kernel(std::size_t diameter) {
	bool_image_kernel kernel(make_ndsize(diameter, diameter));
	auto center = make_ndptrdiff(diameter / 2, diameter / 2);
	std::size_t max_radius_sq = sq(diameter / 2);
	for(auto it = kernel.begin(); it != kernel.end(); ++it) {
		auto coord = it.coordinates();
		std::size_t radius_sq = sq(coord[0] - center[0]) + sq(coord[1] - center[1]);
		*it = (radius_sq <= max_radius_sq);
	}
	return kernel;
}


bool_image_kernel box_image_kernel(std::size_t diameter) {
	bool_image_kernel kernel(make_ndsize(diameter, diameter));
	std::fill(kernel.begin(), kernel.end(), true);
	return kernel;
}


}
