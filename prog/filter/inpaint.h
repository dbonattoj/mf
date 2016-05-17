#ifndef PROG_INPAINT_FILTER_H_
#define PROG_INPAINT_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/masked_elem.h>
#include <mf/color.h>
#include "../support/common.h"


class inpaint_filter : public mf::flow::filter {
public:
	input_type<2, mf::masked_elem<mf::rgb_color>> input;
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
		
	inpaint_filter(mf::flow::filter_node& nd, std::size_t radius) :
		mf::flow::filter(nd), radius_(radius),
		input(*this), output(*this) { }

private:
	std::size_t radius_;

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
