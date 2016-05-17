#ifndef PROG_DEPTH_MAP_IMPROVE_FILTER_H_
#define PROG_DEPTH_MAP_IMPROVE_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/masked_elem.h>
#include "../support/common.h"


class depth_map_improve_filter : public mf::flow::filter {
public:
	using depth_type = std::uint8_t;
	using masked_depth_type = mf::masked_elem<std::uint8_t>;

	input_type<2, masked_depth_type> input;
	output_type<2, masked_depth_type> output;
		
	depth_map_improve_filter(mf::flow::filter_node& nd, std::size_t kernel_diameter = 3);

private:
	std::size_t kernel_diameter_;

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
