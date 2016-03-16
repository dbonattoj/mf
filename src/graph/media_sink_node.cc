#include "media_sink_node.h"
#include <iostream>

namespace mf {

void media_sink_node::setup_graph() {
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_setup_();
}


void media_sink_node::pull_next_frame() {
	std::cout << "frame " << time_ + 1 << "..." << std::endl;
	this->pull(time_ + 1);
}


}