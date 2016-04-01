#include "depth_converter.h"
#include <algorithm>

namespace mf { namespace node {

void depth_converter::process_() {
	std::transform(
		input.view().begin(),
		input.view().end(),
		output.view().begin(),
		[&](mono_color col) -> Eigen_scalar {
			Eigen_scalar d = Eigen_scalar(col.intensity) / 255.0; // TODO generalize
			return 1.0 / (d/z_near + (1.0-d)/z_far);
		}
	);
}

}}
