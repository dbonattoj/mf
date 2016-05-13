#ifndef PROG_DEPTH_MAP_FILTER_NODE_H_
#define PROG_DEPTH_MAP_FILTER_NODE_H_

#include <mf/flow/async_node.h>
#include <mf/masked_elem.h>

class depth_map_filter_node : public mf::flow::async_node {
public:
	using depth_type = std::uint8_t;
	using masked_depth_type = mf::masked_elem<std::uint8_t>;

	input_type<2, masked_depth_type> input;
	output_type<2, masked_depth_type> output;
		
	depth_map_filter_node(mf::flow::graph& gr, std::size_t kernel_diameter = 3);

private:
	std::size_t kernel_diameter_;

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
