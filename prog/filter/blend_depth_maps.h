#ifndef PROG_BLEND_DEPTH_MAPS_FILTER_H_
#define PROG_BLEND_DEPTH_MAPS_FILTER_H_

#include <mf/filter/filter.h>
#include <mf/color.h>
#include <memory>
#include <mf/masked_elem.h>
#include "../support/common.h"


class blend_depth_maps_filter : public mf::flow::filter {
public:
	struct input_visual {
		input_visual(blend_depth_maps_filter& self) :
			depth_input(self) { }
		
		input_type<2, mf::masked_elem<std::uint8_t>> depth_input;
	};

private:
	std::vector<std::unique_ptr<input_visual>> visuals_;
	
public:
	output_type<2, mf::masked_elem<std::uint8_t>> output;
		
	blend_depth_maps_filter(mf::flow::filter_node& nd) :
		mf::flow::filter(nd), output(*this) { }

	input_visual& add_input_visual() {
		visuals_.emplace_back(new input_visual(*this));
		return *visuals_.back();
	}

protected:
	void setup() override;
	void process(mf::flow::node_job&) override;
};

#endif
