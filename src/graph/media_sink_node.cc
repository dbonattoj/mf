#include "media_sink_node.h"

namespace mf {

void media_sink_node::setup_graph() {
	propagate_offset_(0);
	propagate_output_buffer_durations_();
	propagate_setup_();
}


void media_sink_node::stop_graph() {
	propagate_stop_();
}


void media_sink_node::pull_next_frame() {
	this->pull(time_ + 1);
}


}
