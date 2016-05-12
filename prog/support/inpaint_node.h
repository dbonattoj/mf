#ifndef PROG_INPAINT_NODE_H_
#define PROG_INPAINT_NODE_H_

#include <mf/flow/sync_node.h>
#include <mf/masked_elem.h>
#include <mf/color.h>

class inpaint_node : public mf::flow::sync_node {
public:
	input_type<2, mf::masked_elem<mf::rgb_color>> input;
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
		
	inpaint_node(mf::flow::graph& gr, std::size_t radius) :
		mf::flow::sync_node(gr), radius_(radius),
		input(*this), output(*this) { }

private:
	std::size_t radius_;

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
