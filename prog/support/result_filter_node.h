#ifndef PROG_RESULT_FILTER_NODE_H_
#define PROG_RESULT_FILTER_NODE_H_

#include <mf/flow/sync_node.h>
#include <mf/masked_elem.h>
#include <mf/color.h>

class result_filter_node : public mf::flow::sync_node {
public:
	input_type<2, mf::masked_elem<mf::rgb_color>> input;
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
		
	result_filter_node(mf::flow::graph& gr) :
		mf::flow::sync_node(gr),
		input(*this, 2, 2), output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
