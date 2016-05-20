/*
Author : Tim Lenertz
Date : May 2016

Copyright (c) 2016, Université libre de Bruxelles

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files to deal in the Software without restriction, including the rights to use, copy, modify, merge,
publish the Software, and to permit persons to whom the Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "ndarray.h"
#include "../../src/debug.h"
#include <random>

namespace mf { namespace test {

ndarray<2, int> make_frame(const ndsize<2>& shape, int i) {
	std::mt19937 generator(i);
	std::uniform_int_distribution<int> dist;
	ndarray<2, int> frame(shape);
	for(std::ptrdiff_t y = 0; y < shape[0]; ++y)
	for(std::ptrdiff_t x = 0; x < shape[1]; ++x)
		frame[y][x] = 0;
	frame[0][0] = i;
	return frame;
}


int frame_index(const ndarray_view<2, int>& vw, bool verify) {
	const auto& shp = vw.shape();
	int i = vw[0][0];
	if(verify) {
		if(vw == make_frame(shp, i)) return i;
		else return -1;
	} else {
		return i;
	}
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
