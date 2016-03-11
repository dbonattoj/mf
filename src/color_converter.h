#ifndef MF_COLOR_CONVERTER_H_
#define MF_COLOR_CONVERTER_H_

#include <algorithm>
#include "media_node.h"
#include "color.h"

#include <iostream>

namespace mf {

template<typename Input_color, typename Output_color>
class color_converter : public media_node_sequential {	
public:
	media_node_output<2, Output_color> output;
	media_node_input<2, Input_color> input;

	color_converter(const ndsize<2>& shape) :
	output(*this, shape), input(0) {
		register_input_(input);
		register_output_(output);
	}
	
	void process_() override {
		std::transform(
			input.view()[0].begin(),
			input.view()[0].end(),
			output.view().begin(),
			color_convert<Input_color, Output_color>
		);
	}
};

}

#endif
