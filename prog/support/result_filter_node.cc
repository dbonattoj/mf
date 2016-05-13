#include "result_filter_node.h"
#include <mf/image/image.h>
#include <mf/opencv.h>
#include <mf/ndarray/ndarray_filter.h>

using namespace mf;


void result_filter_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void result_filter_node::process(flow::node_job& job) {
	job.out(output) = job.in(input); 
	
	auto in = job.in(input);
	auto in_full = job.in_full(input);
	auto out = job.out(output);
	
	ndarray<3, bool> kernel(make_ndsize(5, 5, 5));
	std::fill(kernel.begin(), kernel.end(), true);
	
	for(auto it = in.begin(); it != in.end(); ++it) {
		auto coord = it.coordinates();
		if(it->is_null()) {
			auto p = place_kernel_at(in_full, kernel.view(), ndcoord_cat(in_full.time_index(job.time()), coord));
			real r = 0, g = 0, b = 0, c = 0;
			for(const auto& col : p.view_section) {
				if(col.is_null()) continue;
				r += col.elem.r; g += col.elem.g; b += col.elem.b; c += 1.0;
			}
			if(c == 0) out.at(coord) = mf::masked_elem<mf::rgb_color>::null();
			else out.at(coord) = rgb_color( clamp(r/c, 0.0f, 255.0f), clamp(g/c, 0.0f, 255.0f), clamp(b/c, 0.0f, 255.0f) );
		} else {
			out.at(coord) = *it;
		}
	}
}
