#include "ndarray.h"
#include <algorithm>

namespace mf {

ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	ndarray<2, int> frame(shape);
	std::fill(frame.begin(), frame.end(), i);
	return frame;
}

}