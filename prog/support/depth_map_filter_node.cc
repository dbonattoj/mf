#include "depth_map_filter_node.h"
#include <mf/image/image.h>
#include <mf/image/kernel.h>
#include <mf/utility/misc.h>
#include <mf/ndarray/ndarray_view.h>
#include <mf/ndarray/ndarray_filter.h>
#include <mf/ndarray/ndcoord.h>
#include <mf/masked_elem.h>
#include <cstdint>
#include <functional>
#include <algorithm>

using namespace mf;

depth_map_filter_node::depth_map_filter_node(flow::graph& gr, std::size_t kernel_diameter) :
	mf::flow::async_node(gr), 
	kernel_diameter_(kernel_diameter),
	input(*this), output(*this) { }


void depth_map_filter_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void depth_map_filter_node::process(flow::node_job& job) {
	auto in = job.in(input);
	auto out = job.out(output);

	std::vector<depth_type> values;
	values.reserve(sq(kernel_diameter_));

	auto filter = [&](const auto& placement, masked_depth_type& out) {
		if(! placement.view_section.at(placement.section_position).is_null())
			out = placement.view_section.at(placement.section_position);
		
		values.clear();
		for(auto it = placement.view_section.begin(); it != placement.view_section.end(); ++it) {
			auto coord = it.coordinates();
			if(! placement.kernel_section.at(coord)) continue;
			if(placement.view_section.at(coord).is_null()) values.push_back(0);
			values.push_back(placement.view_section.at(coord));
		}
		if(values.size() == 0) {
			out = masked_depth_type::null();
		} else {
			std::ptrdiff_t median_index = values.size() / 2;
			std::nth_element(values.begin(), values.begin() + median_index, values.end());
			out = values[median_index];
			if(out.elem == 0) out.mask = false;
		}
	};

	apply_kernel(filter, in, out, box_image_kernel(kernel_diameter_).view());
}
