#include "media_node.h"
#include "media_node_input.h"
#include "media_node_output.h"

namespace mf {

void media_node::register_input_(detail::media_node_input_base& input) {
	inputs_.push_back(&input);
}

void media_node::register_output_(detail::media_node_output_base& output) {
	outputs_.push_back(&output);
}


}