#ifndef PROG_DOWNSCALE_FILTER_H_
#define PROG_DOWNSCALE_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/color.h>
#include "../support/common.h"


class downscale_filter : public mf::flow::filter {
public:
	input_type<2, mf::rgb_color> input;
	output_type<2, mf::rgb_color> output;
		
	using frame_shape_type = mf::ndsize<2>;

private:
	frame_shape_type output_size_;

public:		
	downscale_filter(mf::flow::filter_node& nd, const frame_shape_type& shp) :
		mf::flow::filter(nd), output_size_(shp),
		input(*this), output(*this) { }

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
