#ifndef PROG_DEPTH_MAP_FILTER_NODE_H_
#define PROG_DEPTH_MAP_FILTER_NODE_H_

#include <mf/flow/sync_node.h>

class depth_map_filter_node : public flow::sync_node {
public:
	input_type<2, std::uint8_t> output;
	output_type<2, std::uint8_t> output;
	parameter_type<camera_type> output_camera;
		
	depth_map_filter_node(mf::flow::graph& gr) :
		mf::flow::sync_node(gr), input(*this), output(*this) { }

protected:
	void setup() override;
)	void process(mf::flow::node_job&) override;
};

#endif
