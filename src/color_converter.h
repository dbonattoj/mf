#ifndef MF_COLOR_CONVERTER_H_
#define MF_COLOR_CONVERTER_H_

#include <algorithm>
#include "media_node.h"
#include "color.h"

#include <iostream>

namespace mf {

template<typename Input_color, typename Output_color>
class color_converter : public media_node<2, Output_color> {
	using base = media_node<2, Output_color>;
	
private:
	media_node_input<2, Input_color> input_;
public:
	color_converter(const ndsize<2>& shape) : base(shape) {
		base::add_input_(input_);
	}
	void process_frame_() override {
		auto out = base::output_.write(1);
		std::transform(
			input_.input_view[0].begin(),
			input_.input_view[0].end(),
			out.begin(),
			color_convert<Input_color, Output_color>
		);
		base::output_.did_write(1);
	}
	
	media_node_input<2, Input_color>& image_input() { return input_; }
};

}

#endif
