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

#include <iostream>

using namespace mf;

depth_map_filter_node::depth_map_filter_node(flow::graph& gr, std::size_t kernel_diameter) :
	mf::flow::sync_node(gr), 
	kernel_diameter_(kernel_diameter),
	input(*this, 5, 5), output(*this) { }


void depth_map_filter_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void depth_map_filter_node::process(flow::node_job& job) {
	auto in_img = to_image(job.in(input));
	auto out_img = to_image(job.out(output));

	auto filter = [](const auto& placement, masked_elem<std::uint8_t>& out) {
		out = placement.view_section.at(placement.section_position);
	};

	apply_kernel(filter, in_img.view(), out_img.view(), disk_image_kernel(kernel_diameter_).view());
/*
	in_img.write_cv_mat_background(0);
	cv::medianBlur(in_img.cv_mat(), in_img.cv_mat(), kernel_diameter_);
	in_img.read_cv_mat_background(0);

	in_img.commit_cv_mat();
	out_img.view() = in_img.view();
*/
}
