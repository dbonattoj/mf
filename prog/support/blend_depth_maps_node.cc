#include "blend_depth_maps_node.h"
#include <algorithm>
#include <cmath>
#include <mf/ndarray/ndarray.h>
#include <vector>
#include <algorithm>


using namespace mf;


void blend_depth_maps_node::setup() {
	output.define_frame_shape(visuals_[0]->depth_input.frame_shape());
}


void blend_depth_maps_node::process(flow::node_job& job) {	
	auto out = job.out(output);

	std::vector<std::uint8_t> values;
	values.reserve(visuals_.size());
	
	for(std::ptrdiff_t y = 0; y < out.shape()[0]; ++y)
	for(std::ptrdiff_t x = 0; x < out.shape()[1]; ++x) {
		values.clear();	
		for(auto&& act_vis : visuals_) {
			auto im = job.in(act_vis->depth_input);
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
			out[y][x] = clamp(out_d, 0.0f, 255.0f);
		}
	}
}
