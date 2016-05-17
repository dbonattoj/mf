#ifndef PROG_RESULT_IMPROVE_FILTER_H_
#define PROG_RESULT_IMPROVE_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/masked_elem.h>
#include <mf/color.h>
#include "../support/common.h"


class result_improve_filter : public mf::flow::filter {
public:
	input_type<2, mf::masked_elem<mf::rgb_color>> input;
	output_type<2, mf::masked_elem<mf::rgb_color>> output;
		
	result_improve_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd),
		input(*this, 2, 2), output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
