#include "ndarray.h"
#include <random>

namespace mf { namespace test {

ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	std::mt19937 generator(i);
	std::uniform_int_distribution<int> dist;
	ndarray<2, int> frame(shape);
	for(int& v : frame) v = dist(generator);
	return frame;
}

bool compare_frames(const ndsize<2>& shape, const ndarray_view<3, int>& frames, const std::vector<int>& is) {
	if(frames.shape().front() != is.size()) return false;
	for(std::ptrdiff_t i = 0; i < is.size(); ++i) {
		auto expected = make_frame(shape, is[i]);
		if(frames[i] != expected) return false;
	}
	return true;
}

}}
