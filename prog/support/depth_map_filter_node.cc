#include "depth_map_filter_node.h"

void depth_map_filter_node::setup() {
	output.define_frame_shape(input.frame_shape());
}

void depth_map_filter_node::process(mf::flow::node_job& job) {
	
}
