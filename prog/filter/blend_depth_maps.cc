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

#include "blend_depth_maps.h"
#include <mf/ndarray/ndarray.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace mf;


void blend_depth_maps_filter::setup() {
	output.define_frame_shape(visuals_[0]->depth_input.frame_shape());
}


void blend_depth_maps_filter::process(flow::node_job& job) {	
	auto out = job.out(output);

	std::vector<std::uint8_t> values;
	values.reserve(visuals_.size());
	
	std::vector<ndarray_view<2, masked_elem<std::uint8_t>>> ins;
	for(const auto& vis : visuals_)
		ins.push_back(job.in(vis->depth_input));
	
	for(std::ptrdiff_t y = 0; y < out.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < out.shape()[1]; ++x) {
		values.clear();	
		
		for(const auto& im : ins) {
			auto i_d = im[y][x];
			if(i_d.is_null()) continue;
			values.push_back(i_d);
		}
		
		if(values.size() == 0) {
			out[y][x] = masked_elem<std::uint8_t>::null();
		} else {
			std::ptrdiff_t median_index = values.size() / 2;
			std::nth_element(values.begin(), values.begin() + median_index, values.end());
			real out_d = values[median_index];
			out[y][x] = clamp(out_d, 0.0, 255.0);
		}
	}
}
